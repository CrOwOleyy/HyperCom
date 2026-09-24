#include "server/net/noise_channel.hpp"

#include "common/crypto/secure_memory.hpp"

namespace hypercom::server {

noise_channel::noise_channel(crypto::x25519_public_key const &static_public,
                             crypto::x25519_secret_key const &static_secret)
    : handshake_{static_public, static_secret}, transport_{}
{}

bool noise_channel::accept_handshake_message(
    std::span<std::uint8_t const> input, std::vector<std::uint8_t> &reply)
{
    if (transport_.has_value()) {
        return false;
    }
    std::vector<std::uint8_t> client_payload;
    if (!handshake_.read_first_message(input, client_payload)) {
        return false;
    }
    if (!handshake_.write_second_message({}, reply)) {
        return false;
    }
    crypto::symmetric_key send_key{};
    crypto::symmetric_key receive_key{};
    bool const exported =
        handshake_.export_transport_keys(send_key, receive_key);
    if (exported) {
        transport_.emplace(send_key, receive_key);
    }
    // The keys don't stay on the stack once copied into the transport.
    crypto::wipe_bytes(send_key);
    crypto::wipe_bytes(receive_key);
    return exported;
}

bool noise_channel::is_established() const
{
    return transport_.has_value();
}

bool noise_channel::open_message(std::span<std::uint8_t const> ciphertext,
                                 std::vector<std::uint8_t> &plaintext)
{
    if (!transport_.has_value()) {
        return false;
    }
    return transport_->decrypt_message(ciphertext, plaintext);
}

bool noise_channel::seal_message(std::span<std::uint8_t const> plaintext,
                                 std::vector<std::uint8_t> &ciphertext)
{
    if (!transport_.has_value()) {
        return false;
    }
    return transport_->encrypt_message(plaintext, ciphertext);
}

} // namespace hypercom::server
