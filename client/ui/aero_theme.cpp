#include "client/ui/aero_theme.hpp"

namespace hypercom::client {
namespace {

// Tout est multiplie par scale, arrondis compris : sinon l'interface grossit
// mais les cadres restent fins et l'ensemble a l'air casse.
//
// Les rayons sont volumineux compares a l'ancienne D.A. -- c'est ce qui donne
// l'impression de galet mouille plutot que de fiche cartonnee.
void apply_geometry(ImGuiStyle &style, float scale)
{
    style.WindowRounding = 16.0f * scale;
    style.ChildRounding = 14.0f * scale;
    style.FrameRounding = 12.0f * scale;
    style.PopupRounding = 14.0f * scale;
    style.ScrollbarRounding = 12.0f * scale;
    style.GrabRounding = 10.0f * scale;
    style.TabRounding = 12.0f * scale;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowPadding = ImVec2{16.0f * scale, 14.0f * scale};
    style.FramePadding = ImVec2{11.0f * scale, 7.0f * scale};
    style.ItemSpacing = ImVec2{10.0f * scale, 9.0f * scale};
    style.ItemInnerSpacing = ImVec2{8.0f * scale, 6.0f * scale};
    style.IndentSpacing = 20.0f * scale;
    style.ScrollbarSize = 14.0f * scale;
    style.GrabMinSize = 12.0f * scale;
}

void apply_surfaces(ImVec4 *colors)
{
    colors[ImGuiCol_Text] = AERO_INK;
    colors[ImGuiCol_TextDisabled] = AERO_INK_MUTED;
    // La fenetre principale est transparente : le degrade et les bulles sont
    // peints en dessous par draw_aero_backdrop.
    colors[ImGuiCol_WindowBg] = ImVec4{0.0f, 0.0f, 0.0f, 0.0f};
    colors[ImGuiCol_ChildBg] = AERO_GLASS;
    colors[ImGuiCol_PopupBg] = AERO_GLASS_STRONG;
    colors[ImGuiCol_Border] = AERO_GLASS_BORDER;
    colors[ImGuiCol_BorderShadow] = ImVec4{0.0f, 0.0f, 0.0f, 0.0f};
    colors[ImGuiCol_FrameBg] = AERO_GLASS_STRONG;
    colors[ImGuiCol_FrameBgHovered] = AERO_GLASS_SUNKEN;
    colors[ImGuiCol_FrameBgActive] = AERO_GLASS_SUNKEN;
    colors[ImGuiCol_TitleBg] = AERO_GLASS_SUNKEN;
    colors[ImGuiCol_TitleBgActive] = AERO_ACCENT;
    colors[ImGuiCol_TitleBgCollapsed] = AERO_GLASS_SUNKEN;
    colors[ImGuiCol_MenuBarBg] = AERO_GLASS_SUNKEN;
    colors[ImGuiCol_ScrollbarBg] = ImVec4{1.0f, 1.0f, 1.0f, 0.18f};
    colors[ImGuiCol_ScrollbarGrab] = ImVec4{1.0f, 1.0f, 1.0f, 0.62f};
    colors[ImGuiCol_ScrollbarGrabHovered] = AERO_ACCENT_HOVER;
    colors[ImGuiCol_ScrollbarGrabActive] = AERO_ACCENT;
}

void apply_controls(ImVec4 *colors)
{
    colors[ImGuiCol_CheckMark] = AERO_ACCENT_TEXT;
    colors[ImGuiCol_SliderGrab] = AERO_ACCENT;
    colors[ImGuiCol_SliderGrabActive] = AERO_ACCENT_ACTIVE;
    colors[ImGuiCol_Button] = ImVec4{1.0f, 1.0f, 1.0f, 0.70f};
    colors[ImGuiCol_ButtonHovered] = AERO_ACCENT_HOVER;
    colors[ImGuiCol_ButtonActive] = AERO_ACCENT;
    colors[ImGuiCol_Header] = ImVec4{1.0f, 1.0f, 1.0f, 0.58f};
    colors[ImGuiCol_HeaderHovered] = AERO_ACCENT_HOVER;
    colors[ImGuiCol_HeaderActive] = AERO_ACCENT;
    colors[ImGuiCol_Separator] = ImVec4{1.0f, 1.0f, 1.0f, 0.72f};
    colors[ImGuiCol_SeparatorHovered] = AERO_ACCENT_HOVER;
    colors[ImGuiCol_SeparatorActive] = AERO_ACCENT;
    colors[ImGuiCol_ResizeGrip] = AERO_GLASS_BORDER;
    colors[ImGuiCol_ResizeGripHovered] = AERO_ACCENT_HOVER;
    colors[ImGuiCol_ResizeGripActive] = AERO_ACCENT;
    colors[ImGuiCol_Tab] = AERO_GLASS_SUNKEN;
    colors[ImGuiCol_TabHovered] = AERO_ACCENT_HOVER;
    colors[ImGuiCol_TableHeaderBg] = AERO_GLASS_SUNKEN;
    colors[ImGuiCol_TableBorderStrong] = AERO_GLASS_BORDER;
    colors[ImGuiCol_TableBorderLight] = ImVec4{1.0f, 1.0f, 1.0f, 0.45f};
    colors[ImGuiCol_TextSelectedBg] = ImVec4{0.310f, 0.808f, 0.769f, 0.55f};
    colors[ImGuiCol_NavHighlight] = AERO_ACCENT;
}

} // namespace

void apply_aero_theme(ImGuiStyle &style, float scale)
{
    apply_geometry(style, scale);
    apply_surfaces(style.Colors);
    apply_controls(style.Colors);
}

void draw_section_heading(char const *label)
{
    ImGui::PushStyleColor(ImGuiCol_Text, AERO_ACCENT_TEXT);
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();
}

void draw_status_dot(bool secure, char const *label)
{
    ImVec4 const tint = secure ? AERO_ACCENT_TEXT : AERO_ALERT;
    ImGui::PushStyleColor(ImGuiCol_Text, tint);
    ImGui::TextUnformatted(secure ? "[ok]" : "[!]");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::TextUnformatted(label);
}

} // namespace hypercom::client
