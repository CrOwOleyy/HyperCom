#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "common/crypto/key_types.hpp"

namespace hypercom::crypto {

constexpr std::uint8_t DM_ENVELOPE_VERSION = 1;

// [u8 version][32 identite expediteur][32 ephemere expediteur][u32 compteur]
constexpr std::size_t DM_ENVELOPE_HEADER_SIZE =
    1 + ED25519_PUBLIC_KEY_SIZE + X25519_PUBLIC_KEY_SIZE + 4;

// Ce que le serveur stocke sans pouvoir l'ouvrir.
//
// L'en-tete circule en clair -- il le faut, le destinataire en a besoin pour
// deriver la cle -- mais il est integralement authentifie comme donnee
// associee. Modifier un seul de ses octets, y compris le compteur, fait echouer
// le dechiffrement. Un serveur ne peut donc ni rejouer un message a un autre
// compteur, ni maquiller l'expediteur.
struct dm_envelope_header {
    std::uint8_t version = DM_ENVELOPE_VERSION;
    ed25519_public_key sender_identity{};
    x25519_public_key sender_ephemeral{};
    std::uint32_t counter = 0;
};

// Le destinataire lit l'en-tete d'abord : c'est lui qui indique quelle cle
// deriver. La lecture est bornee et ne suppose rien du reste.
[[nodiscard]] bool parse_dm_envelope_header(
    std::span<std::uint8_t const> envelope, dm_envelope_header &out);

[[nodiscard]] bool seal_dm_envelope(dm_envelope_header const &header,
                                    symmetric_key const &message_key,
                                    std::span<std::uint8_t const> plaintext,
                                    std::vector<std::uint8_t> &out);

[[nodiscard]] bool open_dm_envelope(std::span<std::uint8_t const> envelope,
                                    symmetric_key const &message_key,
                                    std::vector<std::uint8_t> &plaintext_out);

} // namespace hypercom::crypto
