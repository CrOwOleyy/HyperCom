#pragma once

#include "client/keystore/server_registry.hpp"
#include "client/ui/app_state.hpp"
#include "client/ui/server_slot.hpp"

#include <string>

namespace hypercom::client {

// Builds a slot from a registry entry: resolves its identity (derived
// from the seed, or loaded from an imported file) and prepares the
// connection, without opening it yet.
[[nodiscard]] bool build_slot(server_entry const &entry, app_state const &app,
                              std::unique_ptr<server_slot> &out,
                              std::string &error_out);

// Opens the connection and authenticates, if not already done. A
// server whose warning hasn't been acknowledged is never contacted:
// that's the only guarantee that the user has seen what the operator
// will be able to observe before anything goes out.
[[nodiscard]] bool connect_slot(server_slot &slot, std::string &error_out);

// Adds a server from a hypercom:// line and persists it in the
// encrypted registry.
void add_server_from_invite(app_state &app, server_slot_list &slots);

// Acknowledges the first-connection warning and remembers it in the
// registry, so it's only shown once per server.
void acknowledge_slot_trust(app_state &app, server_slot_list &slots,
                            std::size_t index);

} // namespace hypercom::client
