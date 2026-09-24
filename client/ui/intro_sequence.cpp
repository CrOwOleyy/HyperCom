#include "client/ui/intro_sequence.hpp"

namespace hypercom::client {
namespace {

// The blocks only stagger over a fraction of the phase: the last one
// must have finished appearing before the end of the track, not
// exactly at it.
constexpr double REVEAL_STAGGER_SHARE = 0.45;
constexpr double REVEAL_DURATION_SHARE = 0.50;

} // namespace

void begin_intro(intro_state &state, double track_seconds)
{
    state.active = true;
    state.elapsed_seconds = 0.0;
    // A track shorter than the welcome message would give a
    // zero-length reveal phase, so an interface that pops up all at
    // once.
    state.total_seconds = track_seconds > INTRO_WELCOME_SECONDS + 1.0
                              ? track_seconds
                              : INTRO_FALLBACK_TOTAL_SECONDS;
}

void advance_intro(intro_state &state, double delta_seconds)
{
    if (!state.active) {
        return;
    }
    state.elapsed_seconds += delta_seconds;
    if (state.elapsed_seconds >= state.total_seconds) {
        state.active = false;
    }
}

intro_phase get_intro_phase(intro_state const &state)
{
    if (!state.active) {
        return intro_phase::finished;
    }
    if (state.elapsed_seconds < INTRO_WELCOME_SECONDS) {
        return intro_phase::welcome;
    }
    return intro_phase::reveal;
}

float compute_element_reveal(intro_state const &state, int index, int count)
{
    intro_phase const phase = get_intro_phase(state);
    if (phase == intro_phase::finished) {
        return 1.0f;
    }
    if (phase == intro_phase::welcome) {
        return 0.0f;
    }
    double const span = state.total_seconds - INTRO_WELCOME_SECONDS;
    if (span <= 0.0 || count <= 0) {
        return 1.0f;
    }
    double const stagger =
        span * REVEAL_STAGGER_SHARE / static_cast<double>(count);
    double const local = state.elapsed_seconds - INTRO_WELCOME_SECONDS -
                         stagger * static_cast<double>(index);
    if (local <= 0.0) {
        return 0.0f;
    }
    double const progress = local / (span * REVEAL_DURATION_SHARE);
    return progress >= 1.0 ? 1.0f : static_cast<float>(progress);
}

} // namespace hypercom::client
