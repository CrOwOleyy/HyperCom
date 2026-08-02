#pragma once

#include <cstdint>
#include <string>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"

namespace hypercom::proto {

// Message du jour, pousse a la connexion. Modifiable a chaud, sans redemarrer.
//
// revision evite au client de comparer des chaines pour savoir s'il a deja
// affiche cette annonce.
struct motd_push {
    std::uint64_t revision = 0;
    std::string body;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
