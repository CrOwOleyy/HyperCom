#include "common/protocol/byte_reader.hpp"

#include <algorithm>

namespace hypercom::proto {

byte_reader::byte_reader(std::span<std::uint8_t const> buffer)
    : buffer_{buffer}, offset_{0}
{
}

std::size_t byte_reader::count_remaining_bytes() const
{
    return buffer_.size() - offset_;
}

bool byte_reader::take_slice(std::size_t size,
                             std::span<std::uint8_t const> &out)
{
    if (size > count_remaining_bytes()) {
        return false;
    }
    out = buffer_.subspan(offset_, size);
    offset_ += size;
    return true;
}

bool byte_reader::read_fixed_bytes(std::span<std::uint8_t> destination)
{
    std::span<std::uint8_t const> slice;
    if (!take_slice(destination.size(), slice)) {
        return false;
    }
    std::copy(slice.begin(), slice.end(), destination.begin());
    return true;
}

bool byte_reader::read_length_prefixed(std::vector<std::uint8_t> &out,
                                       std::size_t maximum_length)
{
    std::uint32_t announced_length = 0;
    if (!read_integer(announced_length)) {
        return false;
    }
    std::size_t const length = static_cast<std::size_t>(announced_length);
    // Le plafond est verifie avant l'allocation, pas apres : une trame
    // annoncant 4 Gio ne doit jamais provoquer de reservation memoire.
    if (length > maximum_length || length > count_remaining_bytes()) {
        return false;
    }
    std::span<std::uint8_t const> slice;
    if (!take_slice(length, slice)) {
        return false;
    }
    out.assign(slice.begin(), slice.end());
    return true;
}

} // namespace hypercom::proto
