#include "common/protocol/report_message.hpp"

#include "common/protocol/protocol_limits.hpp"
#include "common/protocol/text_field_codec.hpp"

namespace hypercom::proto {

void report_post_request::write_to(byte_writer &writer) const
{
    writer.write_integer(post_id);
    write_text_field(writer, reason);
}

bool report_post_request::read_from(byte_reader &reader)
{
    return reader.read_integer(post_id)
        && read_text_field(reader, reason, MAX_REPORT_REASON_LENGTH);
}

void report_account_request::write_to(byte_writer &writer) const
{
    writer.write_fixed_bytes(target_pubkey);
    write_text_field(writer, reason);
}

bool report_account_request::read_from(byte_reader &reader)
{
    return reader.read_fixed_bytes(target_pubkey)
        && read_text_field(reader, reason, MAX_REPORT_REASON_LENGTH);
}

} // namespace hypercom::proto
