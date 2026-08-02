#include "common/protocol/dm_fetch_message.hpp"

#include "common/protocol/record_list_codec.hpp"

namespace hypercom::proto {

void dm_fetch_request::write_to(byte_writer &writer) const
{
    writer.write_integer(since_id);
    writer.write_integer(limit);
}

bool dm_fetch_request::read_from(byte_reader &reader)
{
    return reader.read_integer(since_id) && reader.read_integer(limit);
}

void dm_list_response::write_to(byte_writer &writer) const
{
    write_record_list(writer, envelopes);
    writer.write_integer(has_more);
}

bool dm_list_response::read_from(byte_reader &reader)
{
    return read_record_list(reader, envelopes, MAX_DM_BATCH_ITEMS)
        && reader.read_integer(has_more);
}

} // namespace hypercom::proto
