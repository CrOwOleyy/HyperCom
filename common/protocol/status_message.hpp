#pragma once

#include <cstdint>
#include <string>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/error_code.hpp"

namespace hypercom::proto {

// Reponse generique de succes. reference_id porte l'identifiant de l'objet
// cree quand il y en a un (forum, post, commentaire), 0 sinon.
struct status_ok_response {
    std::uint64_t reference_id = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// detail s'adresse a l'humain. Brancher la logique du client sur ce texte
// serait une erreur : il peut changer, contrairement a code.
struct status_error_response {
    error_code code = error_code::internal_error;
    std::string detail;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
