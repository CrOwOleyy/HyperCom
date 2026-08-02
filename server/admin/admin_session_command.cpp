#include "server/admin/admin_session_command.hpp"

#include <charconv>

#include "common/util/unix_clock.hpp"
#include "server/admin/admin_text_format.hpp"

namespace hypercom::server {
namespace {

[[nodiscard]] std::string_view describe_phase(session_phase phase)
{
    switch (phase) {
        case session_phase::awaiting_handshake: return "handshake";
        case session_phase::awaiting_hello:     return "hello";
        case session_phase::awaiting_auth:      return "auth";
        case session_phase::authenticated:      return "authentifiee";
    }
    return "inconnue";
}

// Volontairement sans identite : ni pseudo, ni cle publique. Un administrateur
// qui pourrait lister qui est en ligne, depuis quand, aurait un tableau de
// bord de presence -- exactement l'outil de surveillance que le projet existe
// pour rendre impossible. Il n'y a pas de reglage pour reactiver la colonne :
// ce n'est pas une politique qu'on ouvre au cas par cas, comme
// log_peer_addresses, c'est une fonctionnalite qui n'existe pas.
//
// Ce qui reste sert un besoin operationnel reel sans jamais retomber sur une
// personne : combien de connexions, dans quel etat, depuis combien de temps --
// de quoi reperer un handshake bloque ou un slot de connexion qui traine, pas
// de quoi savoir qui parle au reseau.
[[nodiscard]] std::string list_sessions(admin_context &context)
{
    std::uint64_t const now = util::get_unix_timestamp();
    std::string text = pad_right("DESCR", 6) + pad_right("ETAT", 14)
                       + pad_right("DEPUIS", 14) + "INACTIF\n";
    for (auto const &entry : context.registry.connections) {
        session_state const &session = entry.second->session;
        text += pad_right(std::to_string(entry.first), 6);
        text += pad_right(describe_phase(session.phase), 14);
        text += pad_right(format_duration(now - session.connected_at), 14);
        text += format_duration(now - session.last_activity_at);
        text.push_back('\n');
    }
    if (context.registry.connections.empty()) {
        text += "(aucune connexion)\n";
    }
    return text;
}

[[nodiscard]] std::string close_session(admin_context &context,
                                        std::string const &argument,
                                        std::vector<int> &close_requests)
{
    int descriptor = -1;
    auto const parsed = std::from_chars(
        argument.data(), argument.data() + argument.size(), descriptor);
    if (parsed.ec != std::errc{}) {
        return "usage : sessions close <descripteur>\n";
    }
    if (context.registry.connections.find(descriptor)
        == context.registry.connections.end()) {
        return "descripteur inconnu : " + argument + "\n";
    }
    close_requests.push_back(descriptor);
    context.logger.write_entry(util::log_level::info,
                               "session fermee par l'administration");
    return "session " + argument + " marquee pour fermeture\n";
}

} // namespace

std::string run_sessions_command(admin_context &context,
                                 admin_command const &command,
                                 std::vector<int> &close_requests)
{
    if (command.arguments.empty() || command.arguments.front() == "list") {
        return list_sessions(context);
    }
    if (command.arguments.front() == "close") {
        if (command.arguments.size() < 2) {
            return "usage : sessions close <descripteur>\n";
        }
        return close_session(context, command.arguments[1], close_requests);
    }
    return "usage : sessions [list | close <descripteur>]\n";
}

} // namespace hypercom::server
