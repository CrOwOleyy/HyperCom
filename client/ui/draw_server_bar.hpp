#pragma once

#include "client/ui/app_state.hpp"
#include "client/ui/server_slot.hpp"

namespace hypercom::client {

// Server column, on the left: one button per server, plus the add
// field for an invite line. This is the only way to join a server;
// there is no directory.
void draw_server_bar(app_state &app, server_slot_list &slots, float width);

// Blocking warning before the first exchange with an unknown server.
//
// Returns true as long as the warning occupies the screen: the caller
// must then draw nothing else, and above all send nothing to the
// server.
[[nodiscard]] bool draw_trust_warning(app_state &app, server_slot_list &slots,
                                      float scale);

} // namespace hypercom::client
