#pragma once

#include <cstdint>
#include <vector>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

namespace hypercom::proto {

// Tout ce que le serveur garde d'un message prive.
//
// Le format interne de ciphertext est decrit dans common/crypto/dm_envelope,
// pas ici : le serveur n'a aucune cle pour l'ouvrir, et le tag Poly1305 rend
// toute modification detectable.
//
// Limite connue : il voit quand meme qui ecrit a qui, et quand. La masquer
// demanderait un mix-net -- hors perimetre v1, voir docs/THREAT_MODEL.md.
// Pas d'horodatage : la date d'envoi vit a l'interieur du chiffre, donc le
// serveur ne tient aucun registre horodate des echanges. Le client la retrouve
// en dechiffrant.
struct dm_envelope_record {
    std::uint64_t id = 0;
    wire_public_key sender_pubkey{};
    std::vector<std::uint8_t> ciphertext;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
