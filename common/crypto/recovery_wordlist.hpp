#pragma once

#include <array>
#include <cstddef>
#include <string_view>

namespace hypercom::crypto {

constexpr std::size_t RECOVERY_WORDLIST_SIZE = 2048;

// The BIP-39 English word list, in its canonical order. Index = the
// 11-bit value the word encodes.
extern std::array<std::string_view, RECOVERY_WORDLIST_SIZE> const
    RECOVERY_WORDLIST;

} // namespace hypercom::crypto
