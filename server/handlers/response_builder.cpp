#include "server/handlers/response_builder.hpp"

#include "common/protocol/frame_codec.hpp"
#include "common/protocol/length_prefixed_stream.hpp"
#include "common/protocol/status_message.hpp"

namespace hypercom::server {

bool send_raw_message(client_connection &connection, proto::message_type type,
                      std::span<std::uint8_t const> payload)
{
    std::vector<std::uint8_t> frame;
    if (!proto::encode_frame(type, payload, frame)) {
        return false;
    }
    std::vector<std::uint8_t> sealed;
    if (!connection.channel.seal_message(frame, sealed)) {
        return false;
    }
    std::vector<std::uint8_t> wire;
    proto::append_length_prefixed_message(sealed, wire);
    connection.socket.queue_bytes(wire);
    return true;
}

bool send_status_ok(client_connection &connection, std::uint64_t reference_id)
{
    proto::status_ok_response response;
    response.reference_id = reference_id;
    return send_message(connection, proto::message_type::status_ok, response);
}

bool send_status_error(client_connection &connection, proto::error_code code)
{
    proto::status_error_response response;
    response.code = code;
    // The detail stays the generic label for the code: it must never leak
    // anything about the server's internal state or an account's
    // existence.
    response.detail = std::string{proto::describe_error_code(code)};
    return send_message(connection, proto::message_type::status_error,
                        response);
}

} // namespace hypercom::server
