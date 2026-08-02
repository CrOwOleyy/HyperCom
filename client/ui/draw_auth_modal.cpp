#include "client/ui/draw_auth_modal.hpp"

#include <imgui.h>

#include <string>

#include "client/ui/i18n.hpp"
#include "client/ui/aero_theme.hpp"
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
    ImGui::TextColored(AERO_ACCENT, "%s", tr("auth_welcome", state.current_lang));
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped("%s", tr("auth_desc_1", state.current_lang));
    ImGui::PushStyleColor(ImGuiCol_Text, AERO_INK_MUTED);
    ImGui::TextWrapped("%s", tr("auth_desc_2", state.current_lang));
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Spacing();

    draw_section_heading(tr("auth_handle_heading", state.current_lang));

    bool const submit_pressed =
        ImGui::InputText("##registration_handle",
                         state.registration_handle_input,
                         sizeof(state.registration_handle_input),
                         ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::TextDisabled("%s", tr("auth_handle_hint", state.current_lang));

    ImGui::Spacing();

    if (!state.status_message.empty()) {
        ImVec4 const color = state.status_is_error ? AERO_ALERT
                                                   : AERO_ACCENT;
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextWrapped("%s", state.status_message.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    if (ImGui::Button(tr("auth_btn_register", state.current_lang), ImVec2{-1.0f, 40.0f * scale})
        || submit_pressed) {
        std::string const handle_str = state.registration_handle_input;
        std::string failure;
        if (context.session.register_handle(handle_str, failure)) {
            state.registered = true;
            state.handle = context.session.get_handle();
            state.status_message = tr("auth_success", state.current_lang);
            state.status_is_error = false;
            // Seul endroit qui leve ce drapeau : la sequence d'accueil ne se
            // joue qu'a la creation du compte, jamais aux connexions suivantes.
            state.intro_requested = true;
            refresh_forum_list(context, state);
        } else {
            state.status_message = failure;
            state.status_is_error = true;
        }
    }

    ImGui::End();
}

} // namespace hypercom::client
