#include "server/admin/admin_service.hpp"

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

#include "server/admin/admin_dispatcher.hpp"

namespace hypercom::server {
namespace {

// An admin command comfortably fits within this limit. The cap exists so
// that a silent client sending bytes without ever a newline doesn't grow
// the buffer indefinitely.
constexpr std::size_t MAX_REQUEST_BYTES = 4096;

#if defined(MSG_NOSIGNAL)
constexpr int SEND_FLAGS = MSG_NOSIGNAL;
#else
constexpr int SEND_FLAGS = 0;
#endif

// Returns false when the peer has closed or a definitive error occurs.
[[nodiscard]] bool read_into_buffer(int descriptor, std::string &buffer)
{
    char chunk[1024];
    while (true) {
        auto const received =
            ::recv(descriptor, chunk, static_cast<int>(sizeof(chunk)), 0);
        if (received > 0) {
            if (buffer.size() + static_cast<std::size_t>(received) >
                MAX_REQUEST_BYTES) {
                return false;
            }
            buffer.append(chunk, static_cast<std::size_t>(received));
            continue;
        }
        // 0 = peer closed. Negative = nothing left to read for now, which
        // is the normal case on a non-blocking socket.
        return received != 0;
    }
}

// Returns false if the connection must be closed, true otherwise. Consumes
// the written portion of buffer.
[[nodiscard]] bool flush_buffer(int descriptor, std::string &buffer,
                                bool &has_remaining)
{
    while (!buffer.empty()) {
        auto const sent = ::send(descriptor, buffer.data(), buffer.size(),
                                 SEND_FLAGS);
        if (sent > 0) {
            buffer.erase(0, static_cast<std::size_t>(sent));
            continue;
        }
        has_remaining = !buffer.empty();
        // Negative on a non-blocking socket means "try again later", not
        // "error". We can't tell the difference without errno, and
        // retrying is the safe behavior: at worst the connection gets
        // closed by the peer's own timeout.
        return true;
    }
    has_remaining = false;
    return true;
}

void build_response(admin_connection &connection, admin_context &context,
                    std::vector<int> &close_requests)
{
    std::size_t const newline = connection.input.find('\n');
    if (newline == std::string::npos) {
        return;
    }
    std::string const line = connection.input.substr(0, newline);
    connection.input.clear();
    admin_command command;
    if (!parse_admin_command(line, command)) {
        connection.output = "commande vide ou guillemet non ferme\n";
    } else {
        connection.output =
            execute_admin_command(context, command, close_requests);
    }
    connection.response_ready = true;
}

} // namespace

bool open_admin_service(admin_service &service, std::string const &path,
                        event_loop &loop, std::string &error_out)
{
    if (path.empty()) {
        return true;
    }
    if (!service.listener.open_listener(path, error_out)) {
        return false;
    }
    if (!loop.watch_descriptor(service.listener.get_descriptor(), false,
                               false)) {
        error_out = "socket d'admin non enregistrable dans la boucle";
        return false;
    }
    return true;
}

void accept_admin_connections(admin_service &service, event_loop &loop)
{
    while (true) {
        int const accepted = service.listener.accept_connection();
        if (accepted < 0) {
            return;
        }
        admin_connection connection;
        connection.socket = unique_descriptor{accepted};
        service.connections.emplace(accepted, std::move(connection));
        if (!loop.watch_descriptor(accepted, false, false)) {
            service.connections.erase(accepted);
        }
    }
}

bool owns_admin_descriptor(admin_service const &service, int descriptor)
{
    return service.connections.find(descriptor) != service.connections.end();
}

void service_admin_connection(admin_service &service, event_loop &loop,
                              int descriptor, admin_context &context,
                              std::vector<int> &close_requests)
{
    auto const found = service.connections.find(descriptor);
    if (found == service.connections.end()) {
        return;
    }
    admin_connection &connection = found->second;
    if (!connection.response_ready &&
        !read_into_buffer(descriptor, connection.input)) {
        close_admin_connection(service, loop, descriptor);
        return;
    }
    if (!connection.response_ready) {
        build_response(connection, context, close_requests);
    }
    if (!connection.response_ready) {
        return;
    }
    bool has_remaining = false;
    static_cast<void>(
        flush_buffer(descriptor, connection.output, has_remaining));
    if (!has_remaining) {
        close_admin_connection(service, loop, descriptor);
        return;
    }
    if (!loop.watch_descriptor(descriptor, true, true)) {
        close_admin_connection(service, loop, descriptor);
    }
}

void close_admin_connection(admin_service &service, event_loop &loop,
                            int descriptor)
{
    loop.forget_descriptor(descriptor);
    service.connections.erase(descriptor);
}

} // namespace hypercom::server
