#if defined(_WIN32)
#include <afunix.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#include <cstring>
#include <iostream>
#include <string>

// Administration CLI client.
//
//   hypercom_adminctl [--socket <path>] <command> [arguments...]
//
// Sends a line over the admin socket and copies back the response. Nothing
// more: all the intelligence lives server-side, this binary is just a pipe.
//
// It exists so that administration needs neither socat nor netcat, nor any
// knowledge of how the line protocol is built. You type a command, you
// read a response.

namespace {

constexpr char const *DEFAULT_SOCKET_PATH = "run/hypercom-admin.sock";
constexpr int EXIT_USAGE = 2;
constexpr int EXIT_CONNECTION = 3;

void print_usage()
{
    std::cerr
        << "usage : hypercom_adminctl [--socket <chemin>] <commande> [args]\n"
           "\n"
           "  hypercom_adminctl stats\n"
           "  hypercom_adminctl sessions\n"
           "  hypercom_adminctl sessions close 12\n"
           "  hypercom_adminctl motd set \"maintenance samedi 14h\"\n"
           "  hypercom_adminctl motd clear\n"
           "  hypercom_adminctl backup /sauvegardes/hypercom.db\n"
           "  hypercom_adminctl help\n"
           "\n"
           "Defaut du socket : "
        << DEFAULT_SOCKET_PATH << "\n";
}

// An argument containing spaces gets re-wrapped in quotes: that's what
// parse_admin_command expects on the server side, and it saves the
// administrator from having to think about it themselves.
[[nodiscard]] std::string quote_if_needed(std::string const &argument)
{
    if (argument.find(' ') == std::string::npos &&
        argument.find('\t') == std::string::npos) {
        return argument;
    }
    return "\"" + argument + "\"";
}

[[nodiscard]] int connect_to_socket(std::string const &path)
{
#if defined(_WIN32)
    WSADATA data{};
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        return -1;
    }
#endif
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    if (path.size() >= sizeof(address.sun_path)) {
        return -1;
    }
    std::memcpy(address.sun_path, path.c_str(), path.size() + 1);
    int const handle = static_cast<int>(::socket(AF_UNIX, SOCK_STREAM, 0));
    if (handle < 0) {
        return -1;
    }
    if (::connect(handle, reinterpret_cast<sockaddr const *>(&address),
                  sizeof(address)) != 0) {
#if defined(_WIN32)
        ::closesocket(static_cast<SOCKET>(handle));
#else
        ::close(handle);
#endif
        return -1;
    }
    return handle;
}

[[nodiscard]] bool send_all(int handle, std::string const &payload)
{
    std::size_t offset = 0;
    while (offset < payload.size()) {
        auto const sent = ::send(handle, payload.data() + offset,
                                 payload.size() - offset, 0);
        if (sent <= 0) {
            return false;
        }
        offset += static_cast<std::size_t>(sent);
    }
    return true;
}

void copy_response_to_output(int handle)
{
    char chunk[1024];
    while (true) {
        auto const received =
            ::recv(handle, chunk, static_cast<int>(sizeof(chunk)), 0);
        if (received <= 0) {
            return;
        }
        std::cout.write(chunk, received);
    }
}

} // namespace

int main(int argc, char **argv)
{
    std::string socket_path = DEFAULT_SOCKET_PATH;
    int index = 1;
    if (argc > 2 && std::string{argv[1]} == "--socket") {
        socket_path = argv[2];
        index = 3;
    }
    if (index >= argc) {
        print_usage();
        return EXIT_USAGE;
    }
    std::string line;
    for (; index < argc; ++index) {
        if (!line.empty()) {
            line.push_back(' ');
        }
        line += quote_if_needed(argv[index]);
    }
    line.push_back('\n');
    int const handle = connect_to_socket(socket_path);
    if (handle < 0) {
        std::cerr << "connexion impossible au socket d'admin : " << socket_path
                  << "\nLe serveur tourne-t-il, et le chemin est-il le bon ?\n";
        return EXIT_CONNECTION;
    }
    if (!send_all(handle, line)) {
        std::cerr << "envoi de la commande impossible\n";
        return EXIT_CONNECTION;
    }
    copy_response_to_output(handle);
    std::cout.flush();
    return 0;
}
