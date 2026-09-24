#pragma once

#include <imgui.h>

namespace hypercom::client {

// The Frutiger Aero backdrop: gradient, bubbles, highlights.
//
// Everything is painted directly into an ImDrawList rather than through the
// ImGui style, because ImGui can't do gradients or decorative circles.
//
// The bubbles have NO state at all: their position is a pure function of
// their index and time. No array to keep alive between frames, and
// therefore no mutable global either (rule G4).

constexpr int AERO_BUBBLE_COUNT = 22;

void draw_aero_backdrop(ImDrawList *list, ImVec2 origin, ImVec2 size,
                        float time_seconds);

// Glass pane: translucent fill, light-colored edge, highlight at the top.
void draw_glass_surface(ImDrawList *list, ImVec2 minimum, ImVec2 maximum,
                        float rounding);

// The highlight alone, to lay over a widget already drawn by ImGui.
void draw_gloss_highlight(ImDrawList *list, ImVec2 minimum, ImVec2 maximum,
                          float rounding);

} // namespace hypercom::client
