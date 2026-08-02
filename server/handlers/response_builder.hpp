#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "common/protocol/byte_writer.hpp"
#include "common/protocol/error_code.hpp"
#include "common/protocol/message_type.hpp"
#include "server/net/client_connection.hpp"

namespace hypercom::server {

// Chemin de sortie unique : encadrement, scellement Noise, prefixe de longueur,
// mise en file. Aucun handler n'ecrit sur une socket directement, ce qui rend
// structurellement impossible d'emettre une reponse en clair par megarde.
[[nodiscard]] bool send_raw_message(client_connection &connection,
                                    proto::message_type type,
                                    std::span<std::uint8_t const> payload);

template <typename message_struct>
[[nodiscard]] bool send_message(client_connection &connection,
                                proto::message_type type,
                                message_struct const &message)
{
    std::vector<std::uint8_t> payload;
    proto::byte_writer writer{payload};
    message.write_to(writer);
    return send_raw_message(connection, type, payload);
}

[[nodiscard]] bool send_status_ok(client_connection &connection,
                                  std::uint64_t reference_id);

[[nodiscard]] bool send_status_error(client_connection &connection,
                                     proto::error_code code);

} // namespace hypercom::server
