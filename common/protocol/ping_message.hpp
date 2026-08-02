#pragma once

#include <cstdint>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"

namespace hypercom::proto {

// Le jeton est renvoye tel quel : il permet au client d'apparier reponse et
// requete, et de mesurer une latence sans horloge partagee.
struct ping_request {
    std::uint64_t token = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

struct ping_response {
    std::uint64_t token = 0;
    std::uint64_t server_time = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
