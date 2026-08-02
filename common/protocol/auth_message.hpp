#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

namespace hypercom::proto {

// Separation de domaine : une signature produite ici ne doit valoir que pour
// l'authentification Hypercom. Sans ce prefixe, un serveur malveillant pourrait
// soumettre au client un "defi" qui est en realite le hash d'un message a
// signer dans un autre contexte, et rejouer la signature ailleurs.
constexpr std::string_view AUTH_SIGNATURE_DOMAIN = "hypercom-auth-v1";

struct auth_response {
    wire_signature signature{};

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

struct auth_accepted {
    std::uint64_t user_id = 0;
    std::string handle;
    std::uint64_t server_time = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// Construit les octets exacts a signer : domaine, puis nonce, puis cle publique
// annoncee. Client et serveur passent tous deux par ici -- c'est le seul moyen
// d'etre certain qu'ils signent et verifient la meme chose.
void build_auth_signing_input(wire_nonce const &nonce,
                              wire_public_key const &client_pubkey,
                              std::vector<std::uint8_t> &out);

} // namespace hypercom::proto
