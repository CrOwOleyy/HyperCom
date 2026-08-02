#include "client/ui/ui_scale.hpp"

#include <filesystem>

#include <imgui.h>

#include <GLFW/glfw3.h>

namespace hypercom::client {
namespace {

[[nodiscard]] float clamp_user_zoom(float value)
{
    if (value < MIN_USER_ZOOM) {
        return MIN_USER_ZOOM;
    }
    if (value > MAX_USER_ZOOM) {
        return MAX_USER_ZOOM;
    }
    return value;
}

// Une police vectorielle est cherchee dans le systeme plutot qu'embarquee :
// embarquer un fichier de police ajouterait un binaire de plusieurs centaines
// de kilo-octets au depot, pour un gain nul quand le systeme en fournit deja.
// L'absence totale de candidat n'est pas une erreur, seulement un repli.
[[nodiscard]] char const *find_readable_font_path()
{
    static char const *const candidates[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu/DejaVuSans.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/System/Library/Fonts/SFNS.ttf",
    };
    for (char const *const candidate : candidates) {
        std::error_code failure;
        if (std::filesystem::exists(candidate, failure)) {
            return candidate;
        }
    }
    return nullptr;
}

// Sous X11, sous WSLg et sur beaucoup de configurations Linux, le facteur de
// contenu rapporte par le systeme vaut 1.0 meme devant un ecran tres dense.
// La resolution sert alors de second indice : elle n'est pas fiable non plus,
// mais elle rattrape le cas le plus penible -- une interface minuscule sur un
// grand ecran, sans aucun signal DPI pour la corriger.
[[nodiscard]] float infer_scale_from_resolution(float reported_scale)
{
    if (reported_scale >= 1.25f) {
        return reported_scale;
    }
    GLFWmonitor *const monitor = glfwGetPrimaryMonitor();
    if (monitor == nullptr) {
        return reported_scale;
    }
    GLFWvidmode const *const mode = glfwGetVideoMode(monitor);
    if (mode == nullptr) {
        return reported_scale;
    }
    if (mode->width >= 3000) {
        return 1.80f;
    }
    if (mode->width >= 2400) {
        return 1.40f;
    }
    if (mode->width >= 1900) {
        return 1.15f;
    }
    return reported_scale;
}

} // namespace

void detect_display_scale(GLFWwindow *window, ui_scale_state &state)
{
    float horizontal = 1.0f;
    float vertical = 1.0f;
    if (window != nullptr) {
        glfwGetWindowContentScale(window, &horizontal, &vertical);
    }
    float reported = horizontal > vertical ? horizontal : vertical;
    if (reported < 0.1f) {
        reported = 1.0f;
    }
    state.display_scale = infer_scale_from_resolution(reported);
    state.font_rebuild_needed = true;
}

float compute_effective_scale(ui_scale_state const &state)
{
    float const combined = state.display_scale * state.user_zoom;
    if (combined < 0.6f) {
        return 0.6f;
    }
    if (combined > 4.0f) {
        return 4.0f;
    }
    return combined;
}

bool handle_zoom_input(ui_scale_state &state)
{
    ImGuiIO const &io = ImGui::GetIO();
    if (!io.KeyCtrl) {
        return false;
    }
    float const previous = state.user_zoom;
    if (ImGui::IsKeyPressed(ImGuiKey_Equal)
        || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd)) {
        state.user_zoom = clamp_user_zoom(previous + USER_ZOOM_STEP);
    } else if (ImGui::IsKeyPressed(ImGuiKey_Minus)
               || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract)) {
        state.user_zoom = clamp_user_zoom(previous - USER_ZOOM_STEP);
    } else if (ImGui::IsKeyPressed(ImGuiKey_0)
               || ImGui::IsKeyPressed(ImGuiKey_Keypad0)) {
        state.user_zoom = 1.0f;
    } else if (io.MouseWheel < -0.01f || io.MouseWheel > 0.01f) {
        state.user_zoom =
            clamp_user_zoom(previous + io.MouseWheel * USER_ZOOM_STEP);
    }
    float const change = state.user_zoom - previous;
    if (change > -0.001f && change < 0.001f) {
        return false;
    }
    state.font_rebuild_needed = true;
    return true;
}

void rebuild_scaled_font(ui_scale_state &state)
{
    ImGuiIO &io = ImGui::GetIO();
    float const scale = compute_effective_scale(state);
    float const pixel_size = BASE_FONT_SIZE * scale;
    io.Fonts->Clear();
    char const *const font_path = find_readable_font_path();
    ImFont *loaded = nullptr;
    if (font_path != nullptr) {
        // GetGlyphRangesDefault couvre le latin-1, donc les accents francais.
        // La police bitmap integree, elle, s'arrete a l'ASCII.
        loaded = io.Fonts->AddFontFromFileTTF(
            font_path, pixel_size, nullptr, io.Fonts->GetGlyphRangesDefault());
    }
    if (loaded == nullptr) {
        ImFontConfig config;
        config.SizePixels = pixel_size;
        io.Fonts->AddFontDefault(&config);
    }
    io.Fonts->Build();
    // La police est desormais rasterisee a la bonne taille : tout facteur
    // supplementaire ne ferait que la rendre floue.
    io.FontGlobalScale = 1.0f;
    state.applied_scale = scale;
    state.font_rebuild_needed = false;
}

void draw_zoom_controls(ui_scale_state &state, ui_state &ui_state_ref)
{
    ImGui::TextDisabled("%s", tr("zoom_label", ui_state_ref.current_lang));
    ImGui::SameLine();
    if (ImGui::SmallButton("-")) {
        state.user_zoom = clamp_user_zoom(state.user_zoom - USER_ZOOM_STEP);
        state.font_rebuild_needed = true;
    }
    ImGui::SameLine();
    ImGui::Text("%d%%",
                static_cast<int>(compute_effective_scale(state) * 100.0f));
    ImGui::SameLine();
    if (ImGui::SmallButton("+")) {
        state.user_zoom = clamp_user_zoom(state.user_zoom + USER_ZOOM_STEP);
        state.font_rebuild_needed = true;
    }
    ImGui::SameLine();
}

} // namespace hypercom::client
