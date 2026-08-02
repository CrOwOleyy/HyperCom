#include "client/ui/bubble_reveal.hpp"

#include <imgui.h>

namespace hypercom::client {
namespace {

constexpr float REVEAL_RISE_PIXELS = 46.0f;
constexpr float BACK_OVERSHOOT = 1.70158f;

[[nodiscard]] float clamp_unit(float value)
{
    if (value < 0.0f) {
        return 0.0f;
    }
    return value > 1.0f ? 1.0f : value;
}

} // namespace

float ease_out_cubic(float progress)
{
    float const inverted = 1.0f - clamp_unit(progress);
    return 1.0f - inverted * inverted * inverted;
}

float ease_out_back(float progress)
{
    float const inverted = clamp_unit(progress) - 1.0f;
    return 1.0f + (BACK_OVERSHOOT + 1.0f) * inverted * inverted * inverted
           + BACK_OVERSHOOT * inverted * inverted;
}

void begin_bubble_reveal(float progress, float scale)
{
    float const clamped = clamp_unit(progress);
    // L'opacite monte plus vite que la position : le bloc est deja lisible
    // pendant qu'il finit de se placer, ce qui evite l'impression de lenteur.
    float const opacity = ease_out_cubic(clamped * 1.45f);
    float const placement = ease_out_back(clamped);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha,
                        ImGui::GetStyle().Alpha * opacity);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY()
                         + (1.0f - placement) * REVEAL_RISE_PIXELS * scale);
}

void end_bubble_reveal()
{
    ImGui::PopStyleVar();
}

} // namespace hypercom::client
