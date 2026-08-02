#include "common/protocol/prekey_publish_message.hpp"

namespace hypercom::proto {

void prekey_publish_request::write_to(byte_writer &writer) const
{
    writer.write_fixed_bytes(prekey);
    writer.write_fixed_bytes(signature);
}

bool prekey_publish_request::read_from(byte_reader &reader)
{
    return reader.read_fixed_bytes(prekey)
        && reader.read_fixed_bytes(signature);
}

void build_prekey_signing_input(wire_public_key const &identity_pubkey,
                                wire_public_key const &prekey,
                                std::vector<std::uint8_t> &out)
{
    out.clear();
    out.reserve(PREKEY_SIGNATURE_DOMAIN.size() + identity_pubkey.size()
                + prekey.size());
    byte_writer writer{out};
    writer.write_fixed_bytes({reinterpret_cast<std::uint8_t const *>(
                                  PREKEY_SIGNATURE_DOMAIN.data()),
                              PREKEY_SIGNATURE_DOMAIN.size()});
    writer.write_fixed_bytes(identity_pubkey);
    writer.write_fixed_bytes(prekey);
}

} // namespace hypercom::proto
