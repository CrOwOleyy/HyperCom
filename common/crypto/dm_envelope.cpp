#include "common/crypto/dm_envelope.hpp"

#include <algorithm>
#include <array>

#include <sodium.h>

namespace hypercom::crypto {
namespace {

// Le nonce est nul, et c'est sur -- mais uniquement parce que la cle est
// unique par message : le cliquet de dm_message_chain n'en produit jamais deux
// fois la meme. Reutiliser une cle avec un nonce fixe serait catastrophique, et
// c'est pourquoi seal_dm_envelope n'accepte pas de cle de session, seulement
// une cle de message.
constexpr std::array<std::uint8_t, XCHACHA_NONCE_SIZE> ZERO_NONCE{};

void serialize_header(dm_envelope_header const &header,
                      std::vector<std::uint8_t> &out)
{
    out.push_back(header.version);
    out.insert(out.end(), header.sender_identity.begin(),
               header.sender_identity.end());
    out.insert(out.end(), header.sender_ephemeral.begin(),
               header.sender_ephemeral.end());
    for (std::size_t index = 0; index < 4; ++index) {
        out.push_back(
            static_cast<std::uint8_t>((header.counter >> (index * 8U)) & 0xFFU));
    }
}

} // namespace

bool parse_dm_envelope_header(std::span<std::uint8_t const> envelope,
                              dm_envelope_header &out)
{
    if (envelope.size() < DM_ENVELOPE_HEADER_SIZE + AEAD_TAG_SIZE) {
        return false;
    }
    if (envelope[0] != DM_ENVELOPE_VERSION) {
        return false;
    }
    out.version = envelope[0];
    std::size_t offset = 1;
    std::copy_n(envelope.begin() + static_cast<std::ptrdiff_t>(offset),
                out.sender_identity.size(), out.sender_identity.begin());
    offset += out.sender_identity.size();
    std::copy_n(envelope.begin() + static_cast<std::ptrdiff_t>(offset),
                out.sender_ephemeral.size(), out.sender_ephemeral.begin());
    offset += out.sender_ephemeral.size();
    out.counter = 0;
    for (std::size_t index = 0; index < 4; ++index) {
        out.counter |= static_cast<std::uint32_t>(envelope[offset + index])
                       << (index * 8U);
    }
    return true;
}

bool seal_dm_envelope(dm_envelope_header const &header,
                      symmetric_key const &message_key,
                      std::span<std::uint8_t const> plaintext,
                      std::vector<std::uint8_t> &out)
{
    std::vector<std::uint8_t> serialized_header;
    serialized_header.reserve(DM_ENVELOPE_HEADER_SIZE);
    serialize_header(header, serialized_header);
    std::vector<std::uint8_t> sealed(plaintext.size() + AEAD_TAG_SIZE);
    unsigned long long written = 0;
    if (crypto_aead_xchacha20poly1305_ietf_encrypt(
            sealed.data(), &written, plaintext.data(), plaintext.size(),
            serialized_header.data(), serialized_header.size(), nullptr,
            ZERO_NONCE.data(), message_key.data())
        != 0) {
        return false;
    }
    sealed.resize(static_cast<std::size_t>(written));
    out = std::move(serialized_header);
    out.insert(out.end(), sealed.begin(), sealed.end());
    return true;
}

bool open_dm_envelope(std::span<std::uint8_t const> envelope,
                      symmetric_key const &message_key,
                      std::vector<std::uint8_t> &plaintext_out)
{
    dm_envelope_header header{};
    if (!parse_dm_envelope_header(envelope, header)) {
        return false;
    }
    auto const associated_data = envelope.first(DM_ENVELOPE_HEADER_SIZE);
    auto const body = envelope.subspan(DM_ENVELOPE_HEADER_SIZE);
    std::vector<std::uint8_t> decoded(body.size() - AEAD_TAG_SIZE);
    unsigned long long written = 0;
    if (crypto_aead_xchacha20poly1305_ietf_decrypt(
            decoded.data(), &written, nullptr, body.data(), body.size(),
            associated_data.data(), associated_data.size(), ZERO_NONCE.data(),
            message_key.data())
        != 0) {
        return false;
    }
    decoded.resize(static_cast<std::size_t>(written));
    plaintext_out = std::move(decoded);
    return true;
}

} // namespace hypercom::crypto
