#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace hypercom::client {

// Ligne d'invitation collable : hypercom://hote:port#cle_hex
//
// C'est le seul moyen de rejoindre un serveur : il n'existe aucun annuaire, on
// recoit ce lien de quelqu'un en qui on a confiance. Rien n'y est secret -- la
// cle du serveur est publique par nature -- mais elle doit arriver par un canal
// de confiance, sinon l'epinglage ne protege plus de rien.
//
// Le fragment (#) porte la cle par convention : c'est la partie d'une URL qui
// n'est jamais transmise a un serveur web, ce qui rappelle qu'elle n'a rien a
// faire ailleurs que dans le client.
struct invite_link {
    std::string host;
    std::uint16_t port = 0;
    std::string server_key_hex;
};

[[nodiscard]] bool parse_invite_link(std::string_view text, invite_link &out,
                                     std::string &error_out);

[[nodiscard]] std::string format_invite_link(std::string_view host,
                                             std::uint16_t port,
                                             std::string_view server_key_hex);

} // namespace hypercom::client
