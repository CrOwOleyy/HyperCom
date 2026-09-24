#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>

namespace hypercom::proto {

// Integers travel little-endian, regardless of the machine.
// The conversion is done byte by byte: independent of the host's
// endianness and alignment, and without pointer arithmetic.

template <typename T>
[[nodiscard]] bool load_little_endian(std::span<std::uint8_t const> source,
                                      T &out)
{
    static_assert(std::is_unsigned_v<T>,
                  "le protocole ne transporte que des entiers non signes");
    if (source.size() < sizeof(T)) {
        return false;
    }
    T value = 0;
    for (std::size_t index = 0; index < sizeof(T); ++index) {
        value = static_cast<T>(value |
                               (static_cast<T>(source[index]) << (index * 8U)));
    }
    out = value;
    return true;
}

template <typename T>
[[nodiscard]] bool store_little_endian(T value, std::span<std::uint8_t> target)
{
    static_assert(std::is_unsigned_v<T>,
                  "le protocole ne transporte que des entiers non signes");
    if (target.size() < sizeof(T)) {
        return false;
    }
    for (std::size_t index = 0; index < sizeof(T); ++index) {
        target[index] =
            static_cast<std::uint8_t>((value >> (index * 8U)) & 0xFFU);
    }
    return true;
}

} // namespace hypercom::proto
