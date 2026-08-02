#include "server/net/server_runtime.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <csignal>
#include <sys/signalfd.h>
#endif

#include <memory>

#include "common/util/unix_clock.hpp"
#include "server/handlers/connection_processor.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {
namespace {

constexpr int POLL_INTERVAL_MILLISECONDS = 1000;

#if !defined(_WIN32)
[[nodiscard]] int create_signal_descriptor()
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    if (sigprocmask(SIG_BLOCK, &mask, nullptr) != 0) {
        return -1;
    }
    return signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
}
#endif

[[nodiscard]] bool open_configured_listener(listener_config const &settings,
                                             tcp_listener &listener,
                                             std::string &error_out)
{
    if (!settings.enabled) {
        return true;
    }
    return listener.open_listener(settings.bind_address, settings.port,
                                  error_out);
}

} // namespace

server_runtime::server_runtime(server_config const &config,
                               util::logger &logger,
                               database_handle &database,
                               crypto::x25519_public_key const &static_public,
                               crypto::x25519_secret_key const &static_secret)
    : config_{config}, logger_{logger}, database_{database},
      static_public_{static_public}, static_secret_{static_secret},
      clearnet_listener_{}, onion_listener_{}, loop_{},
      registry_{config.limits.max_connections,
                config.limits.max_connections_per_address,
                {},
                {}},
      limiter_{config.limits.requests_per_minute_per_identity},
      signal_descriptor_{}
{
}

bool server_runtime::start_listeners(std::string &error_out)
{
    if (!loop_.open_loop(error_out)) {
        return false;
    }
    if (!open_configured_listener(config_.clearnet, clearnet_listener_,
                                  error_out)
        || !open_configured_listener(config_.onion, onion_listener_,
                                     error_out)) {
        return false;
    }
    if (clearnet_listener_.get_descriptor() >= 0
        && !loop_.watch_descriptor(clearnet_listener_.get_descriptor(), false,
                                   false)) {
        error_out = "listener clearnet non enregistrable";
        return false;
    }
    if (onion_listener_.get_descriptor() >= 0
        && !loop_.watch_descriptor(onion_listener_.get_descriptor(), false,
                                   false)) {
        error_out = "listener onion non enregistrable";
        return false;
    }
#if !defined(_WIN32)
    signal_descriptor_ = unique_descriptor{create_signal_descriptor()};
    if (signal_descriptor_.get_value() < 0
        || !loop_.watch_descriptor(signal_descriptor_.get_value(), false,
                                   false)) {
        error_out = "signalfd indisponible : arret propre impossible";
        return false;
    }
#endif
    return true;
}

void server_runtime::accept_pending_connections(tcp_listener const &listener)
{
    while (true) {
        std::string peer_address;
        int const accepted = listener.accept_connection(peer_address);
        if (accepted < 0) {
            return;
        }
        auto entry = std::make_unique<client_connection>(client_connection{
            connection_socket{accepted},
            noise_channel{static_public_, static_secret_}, session_state{},
            {}});
        entry->session.peer_address = peer_address;
        entry->session.connected_at = util::get_unix_timestamp();
        entry->session.last_activity_at = entry->session.connected_at;
        // En cas de refus, le unique_ptr est detruit par l'appele et la socket
        // se ferme d'elle-meme : il n'y a rien a fermer ici.
        if (!insert_connection(registry_, std::move(entry))) {
            continue;
        }
        if (!loop_.watch_descriptor(accepted, false, false)) {
            close_connection(accepted);
        }
    }
}

void server_runtime::service_connection(int descriptor, std::uint32_t events)
{
    client_connection *const connection =
        find_connection(registry_, descriptor);
    if (connection == nullptr) {
        return;
    }
    if ((events & (EPOLLHUP | EPOLLERR)) != 0) {
        close_connection(descriptor);
        return;
    }
    handler_context context{config_, logger_, database_, *connection};
    if ((events & EPOLLIN) != 0
        && !process_connection_input(context, limiter_)) {
        close_connection(descriptor);
        return;
    }
    bool has_remaining = false;
    if (!connection->socket.flush_pending_writes(has_remaining)) {
        close_connection(descriptor);
        return;
    }
    // EPOLLOUT n'est demande que s'il reste reellement des octets : sinon la
    // boucle tournerait a vide en permanence.
    if (!loop_.watch_descriptor(descriptor, has_remaining, true)) {
        close_connection(descriptor);
    }
}

void server_runtime::close_connection(int descriptor)
{
    loop_.forget_descriptor(descriptor);
    remove_connection(registry_, descriptor);
}

void server_runtime::sweep_expired_connections()
{
    for (int const descriptor : collect_expired_descriptors(
             registry_, util::get_unix_timestamp(), config_.limits)) {
        close_connection(descriptor);
    }
}

bool server_runtime::run_until_stopped(std::string &error_out)
{
    std::vector<epoll_event> events;
    std::uint64_t last_sweep = util::get_unix_timestamp();
    while (true) {
        if (loop_.wait_for_events(POLL_INTERVAL_MILLISECONDS, events) < 0) {
            error_out = "epoll_wait a echoue";
            return false;
        }
        for (epoll_event const &event : events) {
            int const descriptor = event.data.fd;
            if (descriptor == signal_descriptor_.get_value()) {
                logger_.write_entry(util::log_level::info,
                                    "signal recu, arret du serveur");
                return true;
            }
            if (descriptor == clearnet_listener_.get_descriptor()) {
                accept_pending_connections(clearnet_listener_);
            } else if (descriptor == onion_listener_.get_descriptor()) {
                accept_pending_connections(onion_listener_);
            } else {
                service_connection(descriptor, event.events);
            }
        }
        std::uint64_t const now = util::get_unix_timestamp();
        if (now != last_sweep) {
            sweep_expired_connections();
            last_sweep = now;
        }
    }
}

} // namespace hypercom::server
