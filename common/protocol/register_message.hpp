#pragma once

#include <string>
#include <string_view>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"

namespace hypercom::proto {

// Creation de compte, envoyee apres l'authentification : la signature du defi
// a deja prouve qu'on detient la cle privee, il ne reste qu'a choisir un
// pseudo. Ni email, ni telephone, ni mot de passe.
struct register_request {
    std::string handle;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// ASCII restreint : lettres, chiffres, tiret, souligne. Entre 3 et
// MAX_HANDLE_LENGTH caracteres, sans commencer par un chiffre.
//
// La restriction sert a bloquer les homoglyphes. Un "younes" ecrit en
// cyrillique s'affiche pareil mais designe un autre compte, et ici personne
// n'est la pour arbitrer une usurpation.
[[nodiscard]] bool validate_handle(std::string_view handle);

} // namespace hypercom::proto
