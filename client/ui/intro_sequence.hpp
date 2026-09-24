#pragma once

#include <cstdint>

namespace hypercom::client {

// The welcome sequence, played ONCE, on account creation.
//
//   0 s ........ 6 s          welcome message, over the music
//   6 s ........ end of song  the interface appears block by block, as bubbles
//   after that                normal interface, no more animation
//
// The split follows the track: six seconds of intro, then the rhythmic
// part. If the file's actual duration can't be read, we fall back to
// a fixed value -- the animation must never depend on audio succeeding.
enum class intro_phase : std::uint8_t {
    welcome,
    reveal,
    finished,
};

constexpr double INTRO_WELCOME_SECONDS = 6.0;
constexpr double INTRO_FALLBACK_TOTAL_SECONDS = 12.5;

struct intro_state {
    bool active = false;
    double elapsed_seconds = 0.0;
    double total_seconds = INTRO_FALLBACK_TOTAL_SECONDS;
};

void begin_intro(intro_state &state, double track_seconds);

void advance_intro(intro_state &state, double delta_seconds);

[[nodiscard]] intro_phase get_intro_phase(intro_state const &state);

// Progress 0..1 of block `index` out of `count`. Returns 1 outside the
// sequence, so drawing functions don't need to know whether an intro
// is in progress.
[[nodiscard]] float compute_element_reveal(intro_state const &state, int index,
                                           int count);

} // namespace hypercom::client
