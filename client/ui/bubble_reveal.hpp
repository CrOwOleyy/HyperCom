#pragma once

namespace hypercom::client {

// "Bubble" appearance of a UI block.
//
// ImGui can't rescale a widget after the fact, so a panel can't really
// be made to grow. The illusion rests on three things played together
// -- rising opacity, the block sliding up from below, and a slight
// overshoot at the end that gives the bounce of a bubble breaking the
// surface.

[[nodiscard]] float ease_out_cubic(float progress);

// Overshoots the target before settling back. It's this overshoot that
// reads as "bubble" rather than "fade".
[[nodiscard]] float ease_out_back(float progress);

// Always pair with end_bubble_reveal, even if the block drawn between
// the two returns early.
void begin_bubble_reveal(float progress, float scale);

void end_bubble_reveal();

} // namespace hypercom::client
