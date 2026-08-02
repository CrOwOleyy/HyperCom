#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace hypercom::server {

// Une ligne de commande d'administration, decoupee.
//
// Le format est du texte ligne par ligne, pas du binaire : la CLI doit rester
// lisible et scriptable par quelqu'un qui n'ecrit pas de C++. C'est tout
// l'interet du socket d'admin -- le collaborateur tape des commandes, il n'en
// programme pas.
struct admin_command {
    std::string verb;
    std::vector<std::string> arguments;
};

// Decoupe sur les espaces, en respectant les guillemets doubles : un MOTD
// contient des espaces, et on ne va pas demander a l'administrateur de les
// echapper. Renvoie false sur une ligne vide ou un guillemet non ferme.
[[nodiscard]] bool parse_admin_command(std::string_view line,
                                       admin_command &out);

} // namespace hypercom::server
