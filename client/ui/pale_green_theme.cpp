#include "client/ui/pale_green_theme.hpp"

namespace hypercom::client {
namespace {

// Tout est multiplie par scale, y compris les arrondis : sinon l'interface
// grossit mais les cadres restent fins et l'ensemble a l'air casse.
void apply_geometry(ImGuiStyle &style, float scale)
{
    style.WindowRounding = 2.0f * scale;
    style.ChildRounding = 2.0f * scale;
    style.FrameRounding = 2.0f * scale;
    style.PopupRounding = 2.0f * scale;
    style.ScrollbarRounding = 2.0f * scale;
    style.GrabRounding = 2.0f * scale;
    style.TabRounding = 2.0f * scale;
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowPadding = ImVec2{12.0f * scale, 12.0f * scale};
    style.FramePadding = ImVec2{8.0f * scale, 5.0f * scale};
    style.ItemSpacing = ImVec2{8.0f * scale, 7.0f * scale};
    style.ItemInnerSpacing = ImVec2{6.0f * scale, 5.0f * scale};
    style.IndentSpacing = 18.0f * scale;
    style.ScrollbarSize = 12.0f * scale;
    style.GrabMinSize = 10.0f * scale;
}

void apply_text_and_surfaces(ImVec4 *colors)
{
    colors[ImGuiCol_Text] = PALE_GREEN_INK;
    colors[ImGuiCol_TextDisabled] = PALE_GREEN_INK_MUTED;
    colors[ImGuiCol_WindowBg] = PALE_GREEN_CANVAS;
    colors[ImGuiCol_ChildBg] = PALE_GREEN_PANEL;
    colors[ImGuiCol_PopupBg] = PALE_GREEN_PANEL;
    colors[ImGuiCol_Border] = PALE_GREEN_BORDER;
    colors[ImGuiCol_BorderShadow] = ImVec4{0.0f, 0.0f, 0.0f, 0.0f};
    colors[ImGuiCol_FrameBg] = PALE_GREEN_PANEL;
    colors[ImGuiCol_FrameBgHovered] = PALE_GREEN_SUNKEN;
    colors[ImGuiCol_FrameBgActive] = PALE_GREEN_SUNKEN;
    colors[ImGuiCol_TitleBg] = PALE_GREEN_SUNKEN;
    colors[ImGuiCol_TitleBgActive] = PALE_GREEN_ACCENT;
    colors[ImGuiCol_TitleBgCollapsed] = PALE_GREEN_SUNKEN;
    colors[ImGuiCol_MenuBarBg] = PALE_GREEN_SUNKEN;
    colors[ImGuiCol_ScrollbarBg] = PALE_GREEN_CANVAS;
    colors[ImGuiCol_ScrollbarGrab] = PALE_GREEN_BORDER;
    colors[ImGuiCol_ScrollbarGrabHovered] = PALE_GREEN_ACCENT_HOVER;
    colors[ImGuiCol_ScrollbarGrabActive] = PALE_GREEN_ACCENT;
}

void apply_controls(ImVec4 *colors)
{
    colors[ImGuiCol_CheckMark] = PALE_GREEN_ACCENT;
    colors[ImGuiCol_SliderGrab] = PALE_GREEN_ACCENT;
    colors[ImGuiCol_SliderGrabActive] = PALE_GREEN_ACCENT_ACTIVE;
    colors[ImGuiCol_Button] = PALE_GREEN_SUNKEN;
    colors[ImGuiCol_ButtonHovered] = PALE_GREEN_ACCENT_HOVER;
    colors[ImGuiCol_ButtonActive] = PALE_GREEN_ACCENT_ACTIVE;
    colors[ImGuiCol_Header] = PALE_GREEN_SUNKEN;
    colors[ImGuiCol_HeaderHovered] = PALE_GREEN_ACCENT_HOVER;
    colors[ImGuiCol_HeaderActive] = PALE_GREEN_ACCENT;
    colors[ImGuiCol_Separator] = PALE_GREEN_BORDER;
    colors[ImGuiCol_SeparatorHovered] = PALE_GREEN_ACCENT_HOVER;
    colors[ImGuiCol_SeparatorActive] = PALE_GREEN_ACCENT;
    colors[ImGuiCol_ResizeGrip] = PALE_GREEN_BORDER;
    colors[ImGuiCol_ResizeGripHovered] = PALE_GREEN_ACCENT_HOVER;
    colors[ImGuiCol_ResizeGripActive] = PALE_GREEN_ACCENT;
    colors[ImGuiCol_Tab] = PALE_GREEN_SUNKEN;
    colors[ImGuiCol_TabHovered] = PALE_GREEN_ACCENT_HOVER;
    colors[ImGuiCol_TableHeaderBg] = PALE_GREEN_SUNKEN;
    colors[ImGuiCol_TableBorderStrong] = PALE_GREEN_BORDER;
    colors[ImGuiCol_TableBorderLight] = PALE_GREEN_BORDER;
    colors[ImGuiCol_TextSelectedBg] = PALE_GREEN_ACCENT_HOVER;
    colors[ImGuiCol_NavHighlight] = PALE_GREEN_ACCENT;
}

} // namespace

void apply_pale_green_theme(ImGuiStyle &style, float scale)
{
    apply_geometry(style, scale);
    apply_text_and_surfaces(style.Colors);
    apply_controls(style.Colors);
}

void draw_section_heading(char const *label)
{
    ImGui::PushStyleColor(ImGuiCol_Text, PALE_GREEN_INK_MUTED);
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();
}

void draw_status_dot(bool secure, char const *label)
{
    ImVec4 const tint = secure ? PALE_GREEN_ACCENT_TEXT : PALE_GREEN_ALERT;
    ImGui::PushStyleColor(ImGuiCol_Text, tint);
    ImGui::TextUnformatted(secure ? "[ok]" : "[!]");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::TextUnformatted(label);
}

} // namespace hypercom::client
