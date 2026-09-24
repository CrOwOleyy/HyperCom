#include "common/crypto/dm_message_chain.hpp"

#include "common/crypto/hkdf_sha256.hpp"
#include "common/crypto/secure_memory.hpp"

#include <string_view>

namespace hypercom::crypto {
namespace {

constexpr std::string_view MESSAGE_KEY_INFO = "hypercom-dm-message";
constexpr std::string_view CHAIN_KEY_INFO = "hypercom-dm-chain";

[[nodiscard]] std::span<std::uint8_t const> as_bytes(std::string_view text)
{
    return {reinterpret_cast<std::uint8_t const *>(text.data()), text.size()};
}

} // namespace

dm_message_chain::dm_message_chain(symmetric_key const &initial_chain_key)
    : chain_key_{initial_chain_key}, counter_{0}
{}

bool dm_message_chain::derive_next_message_key(symmetric_key &out)
{
    symmetric_key next_chain_key{};
    if (!expand_key_block(chain_key_, as_bytes(MESSAGE_KEY_INFO), 0x01, out)) {
        return false;
    }
    if (!expand_key_block(chain_key_, as_bytes(CHAIN_KEY_INFO), 0x02,
                          next_chain_key)) {
        wipe_bytes(next_chain_key);
        return false;
    }
    // The old chain key is destroyed here, and nowhere else: this line, and
    // this line alone, is what makes past messages unrecoverable.
    wipe_bytes(chain_key_);
    chain_key_ = next_chain_key;
    wipe_bytes(next_chain_key);
    ++counter_;
    return true;
}

bool dm_message_chain::advance_to_counter(std::uint32_t target,
                                          std::vector<symmetric_key> &skipped)
{
    if (target < counter_) {
        return false;
    }
    std::uint32_t const distance = target - counter_;
    if (distance > MAX_SKIPPED_MESSAGE_KEYS) {
        return false;
    }
    for (std::uint32_t step = 0; step < distance; ++step) {
        symmetric_key message_key{};
        if (!derive_next_message_key(message_key)) {
            wipe_bytes(message_key);
            return false;
        }
        skipped.push_back(message_key);
        wipe_bytes(message_key);
    }
    return true;
}

std::uint32_t dm_message_chain::get_counter() const
{
    return counter_;
}

symmetric_key const &dm_message_chain::get_chain_key() const
{
    return chain_key_;
}

} // namespace hypercom::crypto
