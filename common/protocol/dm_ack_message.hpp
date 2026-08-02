#pragma once

#include <cstdint>
#include <vector>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"

namespace hypercom::proto {

// Accuse de reception : le serveur peut alors supprimer les enveloppes citees.
//
// C'est le mecanisme de retention des DM. Une enveloppe acquittee est effacee,
// pas archivee -- ce qui n'existe plus sur le disque ne peut pas etre saisi.
struct dm_ack_request {
    std::vector<std::uint64_t> envelope_ids;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
