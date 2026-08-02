#include "client/ui/draw_forum_panel.hpp"

#include <imgui.h>

#include "client/net/message_exchange.hpp"
#include "client/ui/i18n.hpp"
#include "client/ui/aero_theme.hpp"
#include "client/ui/ui_actions.hpp"
#include "common/protocol/forum_create_message.hpp"

namespace hypercom::client {
namespace {

void draw_forum_creator(cli_context &context, ui_state &state)
{
    draw_section_heading(tr("forum_create_heading", state.current_lang));
    ImGui::InputText(tr("forum_create_name", state.current_lang), state.forum_name_input,
                     sizeof(state.forum_name_input));
    ImGui::InputText(tr("forum_create_desc", state.current_lang), state.forum_description_input,
                     sizeof(state.forum_description_input));
    if (!ImGui::Button(tr("forum_create_btn", state.current_lang))) {
        return;
    }
    proto::forum_create_request request;
    request.name = state.forum_name_input;
    request.description = state.forum_description_input;
    std::string failure;
    proto::forum_info_response response;
    if (send_typed_message(context.connection,
                           proto::message_type::forum_create_request, request)
        && receive_typed_message(context.connection,
                                 proto::message_type::forum_info_response,
                                 response, failure)) {
        state.forum_name_input[0] = '\0';
        state.forum_description_input[0] = '\0';
        refresh_forum_list(context, state);
        state.status_message = "forum fonde";
        state.status_is_error = false;
    } else {
        state.status_message = failure;
        state.status_is_error = true;
    }
}

} // namespace

void draw_forum_list(cli_context &context, ui_state &state, float list_height)
{
    draw_section_heading(tr("forum_heading", state.current_lang));
    if (ImGui::Button(tr("forum_btn_refresh", state.current_lang))) {
        refresh_forum_list(context, state);
    }
    ImGui::BeginChild("forum_list", ImVec2{0.0f, list_height}, true);
    for (proto::forum_record const &forum : state.forums) {
        bool const selected = forum.id == state.selected_forum_id;
        std::string const label = forum.name + "  ("
                                  + std::to_string(forum.post_count) + ")";
        if (ImGui::Selectable(label.c_str(), selected)) {
            state.selected_forum_id = forum.id;
            refresh_post_list(context, state);
        }
    }
    ImGui::EndChild();
    ImGui::Spacing();
    draw_forum_creator(context, state);
}

void draw_post_list(cli_context &context, ui_state &state, float list_height)
{
    draw_section_heading(tr("thread_list_heading", state.current_lang));
    if (state.selected_forum_id == 0) {
        ImGui::TextDisabled("%s", tr("thread_list_empty", state.current_lang));
        return;
    }
    ImGui::BeginChild("post_list", ImVec2{0.0f, list_height}, true);
    for (proto::post_record const &post : state.posts) {
        bool const selected = post.id == state.selected_post_id;
        std::string const label = post.title + "   @" + post.author_handle;
        if (ImGui::Selectable(label.c_str(), selected)) {
            open_thread(context, state, post.id);
        }
    }
    ImGui::EndChild();
}

void draw_post_composer(cli_context &context, ui_state &state, float input_height)
{
    if (state.selected_forum_id == 0) {
        return;
    }
    draw_section_heading(tr("thread_create_heading", state.current_lang));
    ImGui::InputText(tr("thread_create_title", state.current_lang), state.post_title_input,
                     sizeof(state.post_title_input));
    ImGui::InputTextMultiline(tr("thread_create_body", state.current_lang), state.post_body_input,
                              sizeof(state.post_body_input),
                              ImVec2{0.0f, input_height});
    if (ImGui::Button(tr("thread_create_btn", state.current_lang))) {
        submit_post(context, state);
    }
    ImGui::SameLine();
    ImGui::TextDisabled("%s", tr("thread_create_hint", state.current_lang));
}

void draw_forum_column(cli_context &context, ui_state &state, float column_width)
{
    ImGui::BeginChild("colonne_forums", ImVec2{column_width, 0.0f}, true);
    float const avail_h = ImGui::GetContentRegionAvail().y;
    float const forum_h = std::max(100.0f, avail_h * 0.22f);
    float const post_h  = std::max(120.0f, avail_h * 0.28f);
    float const body_h  = std::max(45.0f, avail_h * 0.12f);

    draw_forum_list(context, state, forum_h);
    ImGui::Spacing();
    draw_post_list(context, state, post_h);
    ImGui::Spacing();
    draw_post_composer(context, state, body_h);
    ImGui::EndChild();
}

} // namespace hypercom::client
