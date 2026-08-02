#pragma once

#include <string>
#include <vector>

#include "client/net/server_connection.hpp"
#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/error_code.hpp"
#include "common/protocol/status_message.hpp"

namespace hypercom::client {

template <typename message_struct>
[[nodiscard]] bool send_typed_message(server_connection &connection,
                                      proto::message_type type,
                                      message_struct const &message)
{
    std::vector<std::uint8_t> payload;
    proto::byte_writer writer{payload};
    message.write_to(writer);
    return connection.send_frame(type, payload);
}

// Attend un type precis. Un status_error est traduit en message lisible : le
// client ne doit jamais silencieusement prendre une erreur pour une reponse.
template <typename message_struct>
[[nodiscard]] bool receive_typed_message(server_connection &connection,
                                         proto::message_type expected,
                                         message_struct &out,
                                         std::string &error_out)
{
    proto::frame_header header{};
    std::vector<std::uint8_t> payload;
    if (!connection.receive_frame(header, payload, error_out)) {
        return false;
    }
    proto::byte_reader reader{payload};
    if (header.type == proto::message_type::status_error) {
        proto::status_error_response failure;
        error_out = failure.read_from(reader)
                        ? "erreur serveur : " + failure.detail
                        : "erreur serveur illisible";
        return false;
    }
    if (header.type != expected) {
        error_out = "reponse inattendue du serveur";
        return false;
    }
    if (!out.read_from(reader)) {
        error_out = "reponse mal formee";
        return false;
    }
    return true;
}

} // namespace hypercom::client
