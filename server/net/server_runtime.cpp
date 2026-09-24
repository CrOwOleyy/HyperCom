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

// Plafond volontaire sur le nombre d'acceptations traitees par appel. Sans
// lui, un flux soutenu de nouvelles connexions viderait tout le backlog TCP
// avant qu'un seul octet des connexions deja authentifiees ne soit lu,
// affamant le service en cours pendant l'attaque. Le listener est
// level-triggered (pas d'EPOLLET) : s'il reste des connexions en attente,
// epoll_wait re-signale immediatement le descripteur au prochain tour, donc
// rien n'est perdu -- seul l'ordre de service redevient equitable.
constexpr int MAX_ACCEPTS_PER_CALL = 32;

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
      address_limiter_{config.limits.requests_per_minute_per_address},
      identity_limiter_{config.limits.requests_per_minute_per_identity},
      rate_tracker_{}, admin_{},
      started_at_{util::get_unix_timestamp()}, signal_descriptor_{}
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
    if (!open_admin_service(admin_, config_.paths.admin_socket_path, loop_,
                            error_out)) {
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

void server_runtime::accept_pending_connections(tcp_listener const &listener,
                                                bool is_clearnet)
{
    for (int count = 0; count < MAX_ACCEPTS_PER_CALL; ++count) {
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
        // Jamais sur l'oignon, meme si log_peer_addresses est actif : Tor
        // relaie en boucle locale, il n'y a de toute facon pas de vraie IP a
        // voir cote client. Sur clearnet, redact_peer_address applique deja la
        // politique de hypercom.conf -- ecrire l'entree ici est ce qui rend
        // cette politique reellement effective plutot qu'un reglage qui ne
        // sert jamais a rien (BRIEF.md 13).
        if (is_clearnet) {
            logger_.write_entry(
                util::log_level::info,
                "connexion acceptee depuis "
                    + std::string{logger_.redact_peer_address(peer_address)});
        }
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
    rate_policy policy{address_limiter_, identity_limiter_, rate_tracker_};
    if ((events & EPOLLIN) != 0
        && !process_connection_input(context, policy)) {
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

void server_runtime::dispatch_event(int descriptor, std::uint32_t events)
{
    if (descriptor == clearnet_listener_.get_descriptor()) {
        accept_pending_connections(clearnet_listener_, true);
        return;
    }
    if (descriptor == onion_listener_.get_descriptor()) {
        accept_pending_connections(onion_listener_, false);
        return;
    }
    if (descriptor == admin_.listener.get_descriptor()) {
        accept_admin_connections(admin_, loop_);
        return;
    }
    if (owns_admin_descriptor(admin_, descriptor)) {
        admin_context context{config_, logger_, database_, registry_,
                              started_at_};
        std::vector<int> close_requests;
        service_admin_connection(admin_, loop_, descriptor, context,
                                 close_requests);
        // Les fermetures demandees par `sessions close` sont appliquees ici,
        // une fois le parcours du registre termine.
        for (int const target : close_requests) {
            close_connection(target);
        }
        return;
    }
    service_connection(descriptor, events);
}

void server_runtime::close_connection(int descriptor)
{
    loop_.forget_descriptor(descriptor);
    remove_connection(registry_, descriptor);
}

void server_runtime::sweep_expired_connections()
{
    std::uint64_t const now = util::get_unix_timestamp();
    for (int const descriptor :
         collect_expired_descriptors(registry_, now, config_.limits)) {
        close_connection(descriptor);
    }
    // Meme balayage pour les fenetres de debit : sans ca, les deux tables
    // grossiraient indefiniment et celle des adresses deviendrait un
    // historique.
    forget_expired_windows(rate_tracker_, now);
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
            dispatch_event(descriptor, event.events);
        }
        std::uint64_t const now = util::get_unix_timestamp();
        if (now != last_sweep) {
            sweep_expired_connections();
            last_sweep = now;
        }
    }
}

} // namespace hypercom::server
