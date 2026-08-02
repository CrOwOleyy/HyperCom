#include "common/protocol/dm_envelope_record.hpp"

namespace hypercom::proto {

void dm_envelope_record::write_to(byte_writer &writer) const
{
    writer.write_integer(id);
    writer.write_fixed_bytes(sender_pubkey);
    writer.write_length_prefixed(ciphertext);
}

bool dm_envelope_record::read_from(byte_reader &reader)
{
    return reader.read_integer(id)
        && reader.read_fixed_bytes(sender_pubkey)
        && reader.read_length_prefixed(ciphertext, MAX_DM_CIPHERTEXT_SIZE);
}

} // namespace hypercom::proto
