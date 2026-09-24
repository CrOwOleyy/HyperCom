#include "server/handlers/connection_processor.hpp"

#include "common/crypto/key_types.hpp"
#include "common/protocol/frame_codec.hpp"
#include "common/protocol/length_prefixed_stream.hpp"
#include "common/protocol/protocol_limits.hpp"
#include "common/util/unix_clock.hpp"
#include "server/handlers/request_router.hpp"
#include "server/handlers/response_builder.hpp"

namespace hypercom::server {
namespace {

// The longest Noise message carries one full application frame plus the
// Poly1305 tag.
constexpr std::size_t MAX_NOISE_MESSAGE_SIZE =
    proto::MAX_FRAME_SIZE + crypto::AEAD_TAG_SIZE;

[[nodiscard]] bool advance_handshake(handler_context &context,
                                     std::span<std::uint8_t const> message)
{
    std::vector<std::uint8_t> reply;
    if (!context.connection.channel.accept_handshake_message(message, reply)) {
        return false;
    }
    std::vector<std::uint8_t> wire;
    proto::append_length_prefixed_message(reply, wire);
    context.connection.socket.queue_bytes(wire);
    context.connection.session.phase = session_phase::awaiting_hello;
    return true;
}

[[nodiscard]] bool
dispatch_application_frame(handler_context &context, rate_policy &policy,
                           std::span<std::uint8_t const> frame)
{
    proto::frame_header header{};
    if (!proto::decode_frame_header(frame, header)) {
        return false;
    }
    // The announced length must match exactly what was decrypted: any
    // discrepancy signals a forged frame or a sender bug.
    if (frame.size() != proto::FRAME_LENGTH_FIELD_SIZE + header.body_size) {
        return false;
    }
    session_state &session = context.connection.session;
    std::uint64_t const now = util::get_unix_timestamp();
    // Address first: it also covers sessions that aren't registered yet,
    // which don't have an identity to cap.
    if (!allow_address_request(policy, session.peer_address, now) ||
        !allow_identity_request(policy, session.user_id, now)) {
        return send_status_error(context.connection,
                                 proto::error_code::rate_limited);
    }
    session.last_activity_at = now;
    proto::byte_reader reader{frame.subspan(proto::FRAME_HEADER_SIZE)};
    return route_message(context, header.type, reader);
}

} // namespace

bool process_connection_input(handler_context &context, rate_policy &policy)
{
    client_connection &connection = context.connection;
    if (!connection.socket.read_available(connection.input_buffer)) {
        return false;
    }
    while (true) {
        std::vector<std::uint8_t> message;
        bool malformed = false;
        if (!proto::extract_length_prefixed_message(connection.input_buffer,
                                                    MAX_NOISE_MESSAGE_SIZE,
                                                    message, malformed)) {
            return !malformed;
        }
        if (!connection.channel.is_established()) {
            if (!advance_handshake(context, message)) {
                return false;
            }
            continue;
        }
        std::vector<std::uint8_t> frame;
        if (!connection.channel.open_message(message, frame)) {
            return false;
        }
        if (!dispatch_application_frame(context, policy, frame)) {
            return false;
        }
    }
}

} // namespace hypercom::server
