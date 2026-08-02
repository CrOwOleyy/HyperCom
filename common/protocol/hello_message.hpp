#pragma once

#include <cstdint>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

namespace hypercom::proto {

// Ouverture de session, envoyee sur le canal DEJA chiffre par Noise.
// Le client annonce sa cle publique d'identite ; aucun secret ne circule.
struct hello_request {
    std::uint16_t protocol_version = PROTOCOL_VERSION;
    wire_public_key client_pubkey{};

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// Reponse du serveur : le defi a signer.
//
// account_exists dit au client s'il doit s'authentifier ou s'enregistrer.
// On ne cherche pas a le cacher : n'importe qui peut deja tester l'existence
// d'une cle publique, et le client a besoin de l'info pour savoir s'il doit
// demander un pseudo.
struct auth_challenge {
    std::uint16_t protocol_version = PROTOCOL_VERSION;
    wire_nonce nonce{};
    std::uint8_t account_exists = 0;
    std::uint64_t server_time = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
