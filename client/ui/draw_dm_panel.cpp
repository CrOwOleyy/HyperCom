#include "client/ui/draw_dm_panel.hpp"

#include <imgui.h>

#include "client/net/message_exchange.hpp"
#include "client/ui/aero_theme.hpp"
#include "client/ui/i18n.hpp"
#include "client/ui/ui_dm_actions.hpp"
#include "common/protocol/profile_set_message.hpp"

namespace hypercom::client {
namespace {

void draw_profile_editor(cli_context &context, ui_state &state, float bio_height)
{
    draw_section_heading(tr("profile_heading", state.current_lang));
    ImGui::InputText(tr("profile_name", state.current_lang), state.display_name_input,
                     sizeof(state.display_name_input));
    ImGui::InputTextMultiline(tr("profile_bio", state.current_lang), state.bio_input, sizeof(state.bio_input),
                              ImVec2{0.0f, bio_height});
    if (!ImGui::Button(tr("profile_btn_save", state.current_lang))) {
        return;
    }
    proto::profile_set_request request;
    request.display_name = state.display_name_input;
    request.bio = state.bio_input;
    std::string failure;
    proto::status_ok_response response;
    if (send_typed_message(context.connection,
                           proto::message_type::profile_set_request, request)
        && receive_typed_message(context.connection,
                                 proto::message_type::status_ok, response,
                                 failure)) {
        state.status_message = "profil enregistre";
        state.status_is_error = false;
    } else {
        state.status_message = failure;
        state.status_is_error = true;
    }
}

void draw_inbox(ui_state const &state, float inbox_height)
{
    ImGui::BeginChild("inbox", ImVec2{0.0f, inbox_height}, true);
    for (decrypted_message const &entry : state.inbox) {
        ImGui::PushStyleColor(ImGuiCol_Text, AERO_INK_MUTED);
        ImGui::TextUnformatted(entry.sender_hex.substr(0, 16).c_str());
        ImGui::PopStyleColor();
        if (entry.readable) {
            ImGui::TextWrapped("%s", entry.text.c_str());
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, AERO_ALERT);
            ImGui::TextWrapped("%s : %s", tr("dm_unreadable", state.current_lang), entry.text.c_str());
            ImGui::PopStyleColor();
        }
        ImGui::Separator();
    }
    ImGui::EndChild();
}

} // namespace

void draw_identity_panel(cli_context &context, ui_state &state, float bio_height)
{
    draw_section_heading(tr("identity_heading", state.current_lang));
    ImGui::Text("@%s", state.handle.c_str());
    ImGui::PushStyleColor(ImGuiCol_Text, AERO_INK_MUTED);
    ImGui::TextWrapped("%s", state.identity_hex.c_str());
    ImGui::PopStyleColor();
    if (ImGui::Button(tr("identity_copy_key", state.current_lang))) {
        ImGui::SetClipboardText(state.identity_hex.c_str());
    }
    ImGui::Spacing();
    draw_profile_editor(context, state, bio_height);
}

void draw_dm_panel(cli_context &context, ui_state &state, float inbox_height, float input_height)
{
    draw_section_heading(tr("dm_heading", state.current_lang));
    draw_status_dot(true, tr("dm_status", state.current_lang));
    ImGui::PushStyleColor(ImGuiCol_Text, AERO_INK_MUTED);
    ImGui::TextWrapped("%s", tr("dm_desc", state.current_lang));
    ImGui::PopStyleColor();
    ImGui::Spacing();
    if (ImGui::Button(tr("dm_btn_prekey", state.current_lang))) {
        publish_own_prekey(context, state);
    }
    ImGui::SameLine();
    if (ImGui::Button(tr("dm_btn_inbox", state.current_lang))) {
        refresh_inbox(context, state);
    }
    draw_inbox(state, inbox_height);
    ImGui::InputText(tr("dm_recipient", state.current_lang), state.dm_recipient_input,
                     sizeof(state.dm_recipient_input));
    ImGui::InputTextMultiline("##dm", state.dm_text_input,
                              sizeof(state.dm_text_input),
                              ImVec2{0.0f, input_height});
    if (ImGui::Button(tr("dm_btn_send", state.current_lang))) {
        submit_direct_message(context, state);
    }
}

void draw_side_column(cli_context &context, ui_state &state, float column_width)
{
    ImGui::BeginChild("colonne_laterale", ImVec2{column_width, 0.0f}, true);
    float const avail_h = ImGui::GetContentRegionAvail().y;
    float const inbox_h = std::max(100.0f, avail_h * 0.25f);
    float const input_h = std::max(40.0f, avail_h * 0.08f);

    draw_identity_panel(context, state, input_h);
    ImGui::Spacing();
    draw_dm_panel(context, state, inbox_h, input_h);
    ImGui::EndChild();
}

} // namespace hypercom::client
