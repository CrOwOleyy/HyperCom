#include "common/crypto/noise_transport.hpp"

namespace hypercom::crypto {

noise_transport::noise_transport(symmetric_key const &send_key,
                                 symmetric_key const &receive_key)
    : send_state_{}, receive_state_{}
{
    send_state_.initialize_key(send_key);
    receive_state_.initialize_key(receive_key);
}

bool noise_transport::encrypt_message(std::span<std::uint8_t const> plaintext,
                                      std::vector<std::uint8_t> &out)
{
    // No associated data in the transport phase: the nonce and the key are
    // enough to tie the message to the session, and there is no plaintext
    // header to authenticate.
    return send_state_.encrypt_with_ad({}, plaintext, out);
}

bool noise_transport::decrypt_message(std::span<std::uint8_t const> ciphertext,
                                      std::vector<std::uint8_t> &out)
{
    return receive_state_.decrypt_with_ad({}, ciphertext, out);
}

} // namespace hypercom::crypto
