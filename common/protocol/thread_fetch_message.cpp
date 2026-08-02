#include "common/protocol/thread_fetch_message.hpp"

#include "common/protocol/record_list_codec.hpp"

namespace hypercom::proto {

void thread_fetch_request::write_to(byte_writer &writer) const
{
    writer.write_integer(post_id);
}

bool thread_fetch_request::read_from(byte_reader &reader)
{
    return reader.read_integer(post_id);
}

void thread_response::write_to(byte_writer &writer) const
{
    post.write_to(writer);
    write_record_list(writer, comments);
    writer.write_integer(truncated);
}

bool thread_response::read_from(byte_reader &reader)
{
    return post.read_from(reader)
        && read_record_list(reader, comments, MAX_THREAD_COMMENTS)
        && reader.read_integer(truncated);
}

} // namespace hypercom::proto
