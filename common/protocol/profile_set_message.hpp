#pragma once

#include <string>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"

namespace hypercom::proto {

// Le profil personnalisable, cote MySpace du projet.
//
// theme_json est stocke et resservi tel quel. Le serveur ne l'interprete pas
// et ne valide que l'UTF-8 et la taille -- c'est donc au client de se defendre
// d'un theme hostile.
struct profile_set_request {
    std::string display_name;
    std::string bio;
    std::string theme_json;
    std::string banner_reference;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
