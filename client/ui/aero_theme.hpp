#pragma once

#include <imgui.h>

namespace hypercom::client {

// Art direction: Frutiger Aero.
//
// Translucent glass, sky-to-aqua gradient, rising bubbles, glossy
// highlights. Green remains the project's identity color, but it shifts
// toward turquoise and gains some light.
//
// Two color families, not to be mixed:
//   AERO_ACCENT*        fills and backgrounds
//   AERO_*_TEXT / INK   text only
//
// Text values are measured, not eyeballed. Translucent glass is a
// classic trap: the background varies under the panel, so contrast was
// checked against the three worst cases (glass, top and bottom of the
// gradient). The minimum obtained is 5.21:1, against a WCAG AA
// threshold of 4.5:1.

// Background: hand-drawn gradient, not a flat color.
constexpr ImVec4 AERO_SKY_TOP{0.749f, 0.914f, 1.000f, 1.0f};
constexpr ImVec4 AERO_SKY_BOTTOM{0.788f, 0.949f, 0.847f, 1.0f};

// Glass.
constexpr ImVec4 AERO_GLASS{1.000f, 1.000f, 1.000f, 0.55f};
constexpr ImVec4 AERO_GLASS_STRONG{1.000f, 1.000f, 1.000f, 0.78f};
constexpr ImVec4 AERO_GLASS_SUNKEN{0.851f, 0.925f, 0.949f, 0.72f};
constexpr ImVec4 AERO_GLASS_BORDER{1.000f, 1.000f, 1.000f, 0.85f};

// Turquoise accent: fills.
constexpr ImVec4 AERO_ACCENT{0.122f, 0.663f, 0.627f, 1.0f};
constexpr ImVec4 AERO_ACCENT_HOVER{0.310f, 0.808f, 0.769f, 1.0f};
constexpr ImVec4 AERO_ACCENT_ACTIVE{0.055f, 0.482f, 0.459f, 1.0f};

// Text.
constexpr ImVec4 AERO_INK{0.063f, 0.188f, 0.220f, 1.0f};
constexpr ImVec4 AERO_INK_MUTED{0.235f, 0.376f, 0.408f, 1.0f};
constexpr ImVec4 AERO_ACCENT_TEXT{0.055f, 0.404f, 0.384f, 1.0f};
constexpr ImVec4 AERO_ALERT{0.612f, 0.129f, 0.129f, 1.0f};
constexpr ImVec4 AERO_WARNING{0.545f, 0.365f, 0.055f, 1.0f};

// Highlights and bubbles.
constexpr ImVec4 AERO_GLOSS{1.000f, 1.000f, 1.000f, 0.45f};
constexpr ImVec4 AERO_BUBBLE{1.000f, 1.000f, 1.000f, 0.20f};
constexpr ImVec4 AERO_BUBBLE_RIM{1.000f, 1.000f, 1.000f, 0.42f};

void apply_aero_theme(ImGuiStyle &style, float scale = 1.0f);

// Section title: small turquoise caption followed by a rule.
void draw_section_heading(char const *label);

// Session status.
//
// The marker is [ok] or [!], spelled out, and the label is always
// shown. The color only confirms: turquoise and red are
// indistinguishable for about 8% of men.
void draw_status_dot(bool secure, char const *label);

} // namespace hypercom::client
