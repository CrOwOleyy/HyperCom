#pragma once

#include "common/protocol/endian_codec.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace hypercom::proto {

// Write-side counterpart to byte_reader.
//
// Unlike the reader, it checks no cap. The reason: it only serializes data
// that we've already built and validated ourselves. The only size check
// happens in encode_frame, which rejects an oversized payload. If you find
// yourself writing network-sourced data here, that's probably a sign the
// split needs rethinking.
class byte_writer {
public:
    explicit byte_writer(std::vector<std::uint8_t> &target);

    template <typename T>
    void write_integer(T value)
    {
        std::size_t const offset = target_.size();
        target_.resize(offset + sizeof(T));
        // The space is exactly sizeof(T), so the write cannot fail.
        static_cast<void>(store_little_endian(
            value, std::span<std::uint8_t>{target_}.subspan(offset)));
    }

    void write_fixed_bytes(std::span<std::uint8_t const> data);

    void write_length_prefixed(std::span<std::uint8_t const> data);

    [[nodiscard]] std::size_t count_written_bytes() const;

private:
    std::vector<std::uint8_t> &target_;
    std::size_t initial_size_;
};

} // namespace hypercom::proto
