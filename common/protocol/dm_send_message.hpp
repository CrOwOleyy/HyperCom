#pragma once

#include <cstdint>
#include <vector>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

namespace hypercom::proto {

// Depot d'une enveloppe dans la boite du destinataire.
//
// Le serveur ne peut pas verifier que ciphertext est bien forme -- s'il le
// pouvait, c'est qu'il pourrait le lire. Il se contente de le stocker.
struct dm_send_request {
    wire_public_key recipient_pubkey{};
    std::vector<std::uint8_t> ciphertext;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
