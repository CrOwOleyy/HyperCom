#include "client/ui/draw_welcome_overlay.hpp"

#include <algorithm>

#include <imgui.h>

#include "client/ui/aero_decorations.hpp"
#include "client/ui/aero_theme.hpp"
#include "client/ui/bubble_reveal.hpp"

namespace hypercom::client {
namespace {

constexpr double CARD_FADE_SECONDS = 0.9;
constexpr char const *WELCOME_TITLE = "Bienvenue dans l'espace HyperCom.";
constexpr char const *WELCOME_SUBTITLE = "Amusez-vous et restez gentils !";

// Monte au debut, redescend a la fin, pleine opacite entre les deux.
[[nodiscard]] float compute_card_opacity(intro_state const &state)
{
    double const elapsed = state.elapsed_seconds;
    double const remaining = INTRO_WELCOME_SECONDS - elapsed;
    if (elapsed < CARD_FADE_SECONDS) {
        return ease_out_cubic(static_cast<float>(elapsed / CARD_FADE_SECONDS));
    }
    if (remaining < CARD_FADE_SECONDS) {
        return ease_out_cubic(
            static_cast<float>(std::max(0.0, remaining) / CARD_FADE_SECONDS));
    }
    return 1.0f;
}

void draw_centered_line(char const *text, float font_scale, ImVec4 tint)
{
    ImGui::SetWindowFontScale(font_scale);
    float const width = ImGui::CalcTextSize(text).x;
    float const available = ImGui::GetContentRegionAvail().x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX()
                         + std::max(0.0f, (available - width) * 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Text, tint);
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
    ImGui::SetWindowFontScale(1.0f);
}

} // namespace

void draw_welcome_overlay(intro_state const &state, float scale)
{
    ImGuiViewport const *const viewport = ImGui::GetMainViewport();
    float const opacity = compute_card_opacity(state);
    float const card_width =
        std::min(720.0f * scale, viewport->WorkSize.x * 0.88f);
    float const card_height =
        std::min(260.0f * scale, viewport->WorkSize.y * 0.6f);
    ImVec2 const position{
        viewport->WorkPos.x + (viewport->WorkSize.x - card_width) * 0.5f,
        viewport->WorkPos.y + (viewport->WorkSize.y - card_height) * 0.5f};
    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(ImVec2{card_width, card_height});
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, opacity);
    ImGuiWindowFlags const flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs;
    ImGui::Begin("accueil_hypercom", nullptr, flags);
    ImDrawList *const list = ImGui::GetWindowDrawList();
    draw_gloss_highlight(list, ImGui::GetWindowPos(),
                         ImVec2{ImGui::GetWindowPos().x + card_width,
                                ImGui::GetWindowPos().y + card_height},
                         ImGui::GetStyle().WindowRounding);
    ImGui::Dummy(ImVec2{0.0f, card_height * 0.18f});
    draw_centered_line(WELCOME_TITLE, 1.55f, AERO_INK);
    ImGui::Dummy(ImVec2{0.0f, 14.0f * scale});
    draw_centered_line(WELCOME_SUBTITLE, 1.15f, AERO_ACCENT_TEXT);
    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace hypercom::client
