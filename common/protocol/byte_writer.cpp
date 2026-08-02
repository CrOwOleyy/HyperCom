#include "common/protocol/byte_writer.hpp"

namespace hypercom::proto {

byte_writer::byte_writer(std::vector<std::uint8_t> &target)
    : target_{target}, initial_size_{target.size()}
{
}

void byte_writer::write_fixed_bytes(std::span<std::uint8_t const> data)
{
    target_.insert(target_.end(), data.begin(), data.end());
}

void byte_writer::write_length_prefixed(std::span<std::uint8_t const> data)
{
    write_integer(static_cast<std::uint32_t>(data.size()));
    write_fixed_bytes(data);
}

std::size_t byte_writer::count_written_bytes() const
{
    return target_.size() - initial_size_;
}

} // namespace hypercom::proto
