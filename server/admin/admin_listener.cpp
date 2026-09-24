#include "server/admin/admin_listener.hpp"

#if defined(_WIN32)
#include <afunix.h>
#include <io.h>
#include <winsock2.h>
#else
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#include <cstring>
#include <filesystem>

namespace hypercom::server {
namespace {

constexpr int LISTEN_BACKLOG = 8;

// sockaddr_un.sun_path is a fixed-size array, with no guaranteed
// termination if it overflows. Rejecting a path that's too long beats
// silently truncating it and listening at the wrong location.
[[nodiscard]] bool build_unix_address(std::string const &path, sockaddr_un &out)
{
    out = {};
    out.sun_family = AF_UNIX;
    if (path.size() >= sizeof(out.sun_path)) {
        return false;
    }
    std::memcpy(out.sun_path, path.c_str(), path.size() + 1);
    return true;
}

// A leftover socket file makes bind() fail with EADDRINUSE even though
// nobody's listening anymore. This happens after an unclean shutdown.
void remove_stale_socket(std::string const &path)
{
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
}

void ensure_parent_directory(std::string const &path)
{
    std::error_code ignored;
    std::filesystem::path const target{path};
    if (target.has_parent_path()) {
        std::filesystem::create_directories(target.parent_path(), ignored);
    }
}

// 0600 after bind. On Windows, AF_UNIX doesn't expose a POSIX mode: the
// protection then relies on the parent directory's ACLs, which is weaker
// and should be understood as such.
[[nodiscard]] bool restrict_socket_permissions(std::string const &path)
{
#if defined(_WIN32)
    static_cast<void>(path);
    return true;
#else
    return ::chmod(path.c_str(), S_IRUSR | S_IWUSR) == 0;
#endif
}

[[nodiscard]] bool make_non_blocking(int descriptor)
{
#if defined(_WIN32)
    u_long mode = 1;
    return ::ioctlsocket(static_cast<SOCKET>(descriptor), FIONBIO, &mode) == 0;
#else
    int const flags = ::fcntl(descriptor, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    return ::fcntl(descriptor, F_SETFL, flags | O_NONBLOCK) == 0;
#endif
}

} // namespace

admin_listener::admin_listener() : descriptor_{}, path_{}
{}

admin_listener::~admin_listener()
{
    if (!path_.empty()) {
        remove_stale_socket(path_);
    }
}

bool admin_listener::open_listener(std::string const &path,
                                   std::string &error_out)
{
    sockaddr_un address{};
    if (!build_unix_address(path, address)) {
        error_out = "chemin de socket d'admin trop long : " + path;
        return false;
    }
    ensure_parent_directory(path);
    remove_stale_socket(path);
    unique_descriptor handle{
        static_cast<int>(::socket(AF_UNIX, SOCK_STREAM, 0))};
    if (handle.get_value() < 0) {
        error_out = "socket AF_UNIX indisponible pour l'admin";
        return false;
    }
    if (::bind(handle.get_value(), reinterpret_cast<sockaddr const *>(&address),
               sizeof(address)) != 0) {
        error_out = "bind du socket d'admin impossible : " + path;
        return false;
    }
    if (!restrict_socket_permissions(path)) {
        error_out =
            "impossible de restreindre le socket d'admin a 0600 : " + path;
        return false;
    }
    if (!make_non_blocking(handle.get_value()) ||
        ::listen(handle.get_value(), LISTEN_BACKLOG) != 0) {
        error_out = "listen() sur le socket d'admin a echoue";
        return false;
    }
    descriptor_ = std::move(handle);
    path_ = path;
    return true;
}

int admin_listener::accept_connection() const
{
    int const accepted =
        static_cast<int>(::accept(descriptor_.get_value(), nullptr, nullptr));
    if (accepted < 0) {
        return -1;
    }
    if (!make_non_blocking(accepted)) {
#if defined(_WIN32)
        ::closesocket(static_cast<SOCKET>(accepted));
#else
        ::close(accepted);
#endif
        return -1;
    }
    return accepted;
}

int admin_listener::get_descriptor() const
{
    return descriptor_.get_value();
}

} // namespace hypercom::server
