#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

namespace hypercom::proto {

constexpr std::string_view PREKEY_SIGNATURE_DOMAIN = "hypercom-prekey-v1";

// Publication de la prekey X25519, signee par la cle d'identite Ed25519.
//
// C'est ce qui permet au serveur de distribuer les prekeys sans pouvoir en
// forger : il sert la signature avec, et le destinataire la verifie contre la
// cle d'identite qu'il connait deja. Le serveur reste un annuaire, jamais une
// autorite.
struct prekey_publish_request {
    wire_public_key prekey{};
    wire_signature signature{};

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// Meme role que build_auth_signing_input : garantir que le signataire et le
// verificateur travaillent sur exactement les memes octets. La cle d'identite
// est incluse pour qu'une prekey signee ne puisse pas etre recollee sur une
// autre identite.
void build_prekey_signing_input(wire_public_key const &identity_pubkey,
                                wire_public_key const &prekey,
                                std::vector<std::uint8_t> &out);

} // namespace hypercom::proto
