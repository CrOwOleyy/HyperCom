#include "common/protocol/dm_send_message.hpp"

namespace hypercom::proto {

void dm_send_request::write_to(byte_writer &writer) const
{
    writer.write_fixed_bytes(recipient_pubkey);
    writer.write_length_prefixed(ciphertext);
}

bool dm_send_request::read_from(byte_reader &reader)
{
    return reader.read_fixed_bytes(recipient_pubkey)
        && reader.read_length_prefixed(ciphertext, MAX_DM_CIPHERTEXT_SIZE);
}

} // namespace hypercom::proto
