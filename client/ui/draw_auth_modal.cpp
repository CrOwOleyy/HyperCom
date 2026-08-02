#include "client/ui/draw_auth_modal.hpp"

#include <imgui.h>

#include <string>

#include "client/ui/pale_green_theme.hpp"
#include "client/ui/ui_actions.hpp"

namespace hypercom::client {

void draw_auth_modal(cli_context &context, ui_state &state, float scale)
{
    ImGuiViewport const *const viewport = ImGui::GetMainViewport();

    // Carte centree
    float const card_w = std::min(480.0f * scale, viewport->WorkSize.x * 0.85f);
    float const card_h = std::min(340.0f * scale, viewport->WorkSize.y * 0.85f);

    ImVec2 const center_pos{
        viewport->WorkPos.x + (viewport->WorkSize.x - card_w) * 0.5f,
        viewport->WorkPos.y + (viewport->WorkSize.y - card_h) * 0.5f};

    ImGui::SetNextWindowPos(center_pos);
    ImGui::SetNextWindowSize(ImVec2{card_w, card_h});

    ImGuiWindowFlags const card_flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("creation_compte_modal", nullptr, card_flags);

    ImGui::Spacing();
    ImGui::TextColored(PALE_GREEN_ACCENT, "BIENVENUE SUR HYPERCOM");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped(
        "Votre clé publique n'a pas encore de compte sur ce serveur.");
    ImGui::PushStyleColor(ImGuiCol_Text, PALE_GREEN_INK_MUTED);
    ImGui::TextWrapped("Choisissez un pseudo pour vous enregistrer et accéder au réseau social.");
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Spacing();

    draw_section_heading("PSEUDO DE COMPTE");

    bool const submit_pressed =
        ImGui::InputText("##registration_handle",
                         state.registration_handle_input,
                         sizeof(state.registration_handle_input),
                         ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::TextDisabled(
        "3 à 32 caractères (lettres, chiffres, tiret, souligné)");

    ImGui::Spacing();

    if (!state.status_message.empty()) {
        ImVec4 const color = state.status_is_error ? PALE_GREEN_ALERT
                                                   : PALE_GREEN_ACCENT;
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextWrapped("%s", state.status_message.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    if (ImGui::Button("S'inscrire et se connecter", ImVec2{-1.0f, 40.0f * scale})
        || submit_pressed) {
        std::string const handle_str = state.registration_handle_input;
        std::string failure;
        if (context.session.register_handle(handle_str, failure)) {
            state.registered = true;
            state.handle = context.session.get_handle();
            state.status_message = "Compte créé avec succès !";
            state.status_is_error = false;
            refresh_forum_list(context, state);
        } else {
            state.status_message = failure;
            state.status_is_error = true;
        }
    }

    ImGui::End();
}

} // namespace hypercom::client
