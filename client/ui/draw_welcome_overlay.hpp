#pragma once

#include "client/ui/intro_sequence.hpp"

namespace hypercom::client {

// The screen for the first six seconds: a centered glass card, two
// lines, nothing else. No button -- nothing is asked of the user
// during this time, they've just created their account.
void draw_welcome_overlay(intro_state const &state, float scale);

} // namespace hypercom::client
