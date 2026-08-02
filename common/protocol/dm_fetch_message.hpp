#pragma once

#include <cstdint>
#include <vector>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/dm_envelope_record.hpp"

namespace hypercom::proto {

// Releve de boite. since_id vaut 0 au premier appel, puis l'identifiant de la
// derniere enveloppe deja recue : le client rattrape son retard sans que le
// serveur ait a memoriser sa position.
struct dm_fetch_request {
    std::uint64_t since_id = 0;
    std::uint16_t limit = MAX_DM_BATCH_ITEMS;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

struct dm_list_response {
    std::vector<dm_envelope_record> envelopes;
    std::uint8_t has_more = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
