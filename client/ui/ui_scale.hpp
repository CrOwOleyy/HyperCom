#pragma once

#include "client/ui/ui_state.hpp"

struct GLFWwindow;

namespace hypercom::client {

// Interface scaling.
//
// The problem this module solves: Dear ImGui's built-in font is a
// 13-pixel bitmap. On a dense or simply large screen it's unreadable,
// and enlarging it with FontGlobalScale only stretches it -- you get
// big and blurry, not big and crisp.
//
// The solution is to RASTERIZE the font at the right size rather than
// zoom an image. The atlas is therefore rebuilt on every scale change,
// and only then: it's an expensive operation that has no business
// being in a render loop.
//
// The final scale combines two factors:
//   display_scale  inferred from the system (DPI, then resolution as
//                  a fallback)
//   user_zoom      set by the user, because no heuristic knows their
//                  distance from the screen or their eyesight

constexpr float MIN_USER_ZOOM = 0.7f;
constexpr float MAX_USER_ZOOM = 2.5f;
constexpr float USER_ZOOM_STEP = 0.1f;

// 17 px rather than the original 13 px: even with no scaling applied
// at all, the default should already be comfortable.
constexpr float BASE_FONT_SIZE = 17.0f;

struct ui_scale_state {
    float display_scale = 1.0f;
    float user_zoom = 1.0f;
    float applied_scale = 0.0f;
    bool font_rebuild_needed = true;
};

void detect_display_scale(GLFWwindow *window, ui_scale_state &state);

[[nodiscard]] float compute_effective_scale(ui_scale_state const &state);

// Ctrl + / Ctrl - / Ctrl 0, and Ctrl + wheel. To call during a frame:
// the rebuild flag is handled on the next pass, between two frames.
[[nodiscard]] bool handle_zoom_input(ui_scale_state &state);

// To call ONLY BETWEEN two frames, never between NewFrame and Render.
void rebuild_scaled_font(ui_scale_state &state);

void draw_zoom_controls(ui_scale_state &state, ui_state &ui_state_ref);

} // namespace hypercom::client
