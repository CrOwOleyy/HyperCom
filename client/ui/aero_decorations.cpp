#include "client/ui/aero_decorations.hpp"

#include <cmath>

#include "client/ui/aero_theme.hpp"

namespace hypercom::client {
namespace {

// Bruit deterministe : meme indice, meme valeur, a chaque image et sur toutes
// les machines. C'est ce qui evite d'avoir a stocker les bulles quelque part.
[[nodiscard]] float hash_unit(int index, int salt)
{
    float const raw =
        std::sin(static_cast<float>(index * 127 + salt * 311) * 0.7548f)
        * 43758.5453f;
    return raw - std::floor(raw);
}

struct bubble_shape {
    ImVec2 center;
    float radius;
};

// Une bulle monte lentement, derive lateralement, et reboucle en haut.
[[nodiscard]] bubble_shape compute_bubble(int index, ImVec2 origin,
                                          ImVec2 size, float time_seconds)
{
    float const radius = 12.0f + hash_unit(index, 4) * 46.0f;
    float const rise_speed = 9.0f + hash_unit(index, 2) * 26.0f;
    float const travel = size.y + radius * 4.0f;
    float const offset =
        std::fmod(time_seconds * rise_speed + hash_unit(index, 3) * travel,
                  travel);
    float const drift =
        std::sin(time_seconds * 0.35f + static_cast<float>(index)) * 26.0f;
    bubble_shape shape;
    shape.radius = radius;
    shape.center.x = origin.x + hash_unit(index, 1) * size.x + drift;
    shape.center.y = origin.y + size.y + radius * 2.0f - offset;
    return shape;
}

void draw_single_bubble(ImDrawList *list, bubble_shape const &shape)
{
    list->AddCircleFilled(shape.center, shape.radius,
                          ImGui::GetColorU32(AERO_BUBBLE), 32);
    list->AddCircle(shape.center, shape.radius,
                    ImGui::GetColorU32(AERO_BUBBLE_RIM), 32,
                    1.5f);
    // Le petit reflet en haut a gauche : c'est lui qui fait lire un volume
    // plutot qu'un simple disque.
    ImVec2 const glint{shape.center.x - shape.radius * 0.34f,
                       shape.center.y - shape.radius * 0.38f};
    list->AddCircleFilled(glint, shape.radius * 0.20f,
                          ImGui::GetColorU32(AERO_GLOSS), 16);
}

} // namespace

void draw_aero_backdrop(ImDrawList *list, ImVec2 origin, ImVec2 size,
                        float time_seconds)
{
    ImVec2 const corner{origin.x + size.x, origin.y + size.y};
    ImU32 const top = ImGui::GetColorU32(AERO_SKY_TOP);
    ImU32 const bottom = ImGui::GetColorU32(AERO_SKY_BOTTOM);
    list->AddRectFilledMultiColor(origin, corner, top, top, bottom, bottom);
    for (int index = 0; index < AERO_BUBBLE_COUNT; ++index) {
        draw_single_bubble(list, compute_bubble(index, origin, size,
                                                time_seconds));
    }
}

void draw_glass_surface(ImDrawList *list, ImVec2 minimum, ImVec2 maximum,
                        float rounding)
{
    list->AddRectFilled(minimum, maximum, ImGui::GetColorU32(AERO_GLASS),
                        rounding);
    draw_gloss_highlight(list, minimum, maximum, rounding);
    list->AddRect(minimum, maximum, ImGui::GetColorU32(AERO_GLASS_BORDER),
                  rounding, 0, 1.5f);
}

void draw_gloss_highlight(ImDrawList *list, ImVec2 minimum, ImVec2 maximum,
                          float rounding)
{
    // Le reflet ne couvre que la moitie haute, et s'eteint vers le bas : c'est
    // la signature visuelle de l'epoque, un plastique brillant eclaire d'en
    // haut.
    float const middle = minimum.y + (maximum.y - minimum.y) * 0.48f;
    ImU32 const bright = ImGui::GetColorU32(AERO_GLOSS);
    ImU32 const clear = ImGui::GetColorU32(ImVec4{1.0f, 1.0f, 1.0f, 0.0f});
    list->PushClipRect(minimum, ImVec2{maximum.x, middle}, true);
    list->AddRectFilled(minimum, ImVec2{maximum.x, middle + 1.0f}, bright,
                        rounding);
    list->PopClipRect();
    list->AddRectFilledMultiColor(ImVec2{minimum.x, middle},
                                  ImVec2{maximum.x, maximum.y}, clear, clear,
                                  ImGui::GetColorU32(
                                      ImVec4{1.0f, 1.0f, 1.0f, 0.10f}),
                                  ImGui::GetColorU32(
                                      ImVec4{1.0f, 1.0f, 1.0f, 0.10f}));
}

} // namespace hypercom::client
