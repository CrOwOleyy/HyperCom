#pragma once

#include <cstdint>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

namespace hypercom::proto {

struct prekey_fetch_request {
    wire_public_key target_pubkey{};

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// Le destinataire verifie la signature avec owner_pubkey avant d'utiliser la
// prekey. Un serveur qui substituerait sa propre prekey pour se placer en
// intercepteur echouerait a produire une signature valide -- il ne possede pas
// la cle d'identite de la personne visee.
struct prekey_bundle_response {
    wire_public_key owner_pubkey{};
    wire_public_key prekey{};
    wire_signature signature{};
    std::uint64_t created_at = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
