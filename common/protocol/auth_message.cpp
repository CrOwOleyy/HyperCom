#include "common/protocol/auth_message.hpp"

#include "common/protocol/text_field_codec.hpp"

namespace hypercom::proto {

void auth_response::write_to(byte_writer &writer) const
{
    writer.write_fixed_bytes(signature);
}

bool auth_response::read_from(byte_reader &reader)
{
    return reader.read_fixed_bytes(signature);
}

void auth_accepted::write_to(byte_writer &writer) const
{
    writer.write_integer(user_id);
    write_text_field(writer, handle);
    writer.write_integer(server_time);
}

bool auth_accepted::read_from(byte_reader &reader)
{
    return reader.read_integer(user_id)
        && read_text_field(reader, handle, MAX_HANDLE_LENGTH)
        && reader.read_integer(server_time);
}

void build_auth_signing_input(wire_nonce const &nonce,
                              wire_public_key const &client_pubkey,
                              std::vector<std::uint8_t> &out)
{
    out.clear();
    out.reserve(AUTH_SIGNATURE_DOMAIN.size() + nonce.size()
                + client_pubkey.size());
    byte_writer writer{out};
    writer.write_fixed_bytes({reinterpret_cast<std::uint8_t const *>(
                                  AUTH_SIGNATURE_DOMAIN.data()),
                              AUTH_SIGNATURE_DOMAIN.size()});
    writer.write_fixed_bytes(nonce);
    writer.write_fixed_bytes(client_pubkey);
}

} // namespace hypercom::proto
