#pragma once

#include "common/protocol/endian_codec.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace hypercom::proto {

// Bounds-checked reader. Everything coming from the network goes through
// here, no exceptions.
//
// Three things to know before modifying it:
//   - it never reads past the buffer, everything goes through take_slice;
//   - it reports failure via the return value, leaving the output untouched;
//   - it never allocates before checking the announced size.
//
// The last rule is the most important one. If you add a method that
// allocates, check the cap BEFORE, not after.
class byte_reader {
public:
    explicit byte_reader(std::span<std::uint8_t const> buffer);

    template <typename T>
    [[nodiscard]] bool read_integer(T &out)
    {
        std::span<std::uint8_t const> slice;
        if (!take_slice(sizeof(T), slice)) {
            return false;
        }
        return load_little_endian(slice, out);
    }

    // For fields of known size: public keys, signatures, nonces.
    [[nodiscard]] bool read_fixed_bytes(std::span<std::uint8_t> destination);

    // Reads [u32 size][bytes]. The size is compared against maximum_length
    // and the remaining buffer before anything gets reserved.
    [[nodiscard]] bool read_length_prefixed(std::vector<std::uint8_t> &out,
                                            std::size_t maximum_length);

    [[nodiscard]] std::size_t count_remaining_bytes() const;

private:
    [[nodiscard]] bool take_slice(std::size_t size,
                                  std::span<std::uint8_t const> &out);

    std::span<std::uint8_t const> buffer_;
    std::size_t offset_;
};

} // namespace hypercom::proto
