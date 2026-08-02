#include "client/ui/draw_thread_panel.hpp"

#include <imgui.h>

#include "client/ui/aero_theme.hpp"
#include "client/ui/i18n.hpp"
#include "client/ui/ui_actions.hpp"

namespace hypercom::client {
namespace {

// Profondeur d'indentation plafonnee a l'affichage. Le serveur borne deja
// l'arbre, mais l'UI ne doit pas dependre de cette borne pour rester lisible :
// au-dela, on cesse simplement de decaler.
constexpr std::uint16_t MAX_VISUAL_DEPTH = 8;

void draw_post_header(ui_state const &state)
{
    ImGui::PushStyleColor(ImGuiCol_Text, AERO_INK);
    ImGui::TextWrapped("%s", state.open_post.title.c_str());
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, AERO_INK_MUTED);
    ImGui::Text("@%s", state.open_post.author_handle.c_str());
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextWrapped("%s", state.open_post.body.c_str());
    ImGui::Spacing();
}

void draw_comment_tree(cli_context &context, ui_state &state)
{
    for (proto::comment_record const &comment : state.comments) {
        std::uint16_t const depth =
            comment.depth > MAX_VISUAL_DEPTH ? MAX_VISUAL_DEPTH
                                             : comment.depth;
        ImGui::Indent(static_cast<float>(depth) * 16.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, AERO_INK_MUTED);
        ImGui::Text("@%s", comment.author_handle.c_str());
        ImGui::PopStyleColor();
        ImGui::TextWrapped("%s", comment.body.c_str());
        ImGui::PushID(static_cast<int>(comment.id));
        if (ImGui::SmallButton(tr("reply_action", state.current_lang))) {
            submit_comment(context, state, comment.id);
        }
        ImGui::PopID();
        ImGui::Unindent(static_cast<float>(depth) * 16.0f);
        ImGui::Spacing();
    }
}

} // namespace

void draw_thread_column(cli_context &context, ui_state &state, float column_width)
{
    ImGui::BeginChild("colonne_fil", ImVec2{column_width, 0.0f}, true);
    if (state.selected_post_id == 0) {
        ImGui::TextDisabled("%s", tr("post_read_empty", state.current_lang));
        ImGui::EndChild();
        return;
    }
    draw_post_header(state);
    draw_section_heading(tr("replies_heading", state.current_lang));

    float const avail_h = ImGui::GetContentRegionAvail().y;
    float const input_h = std::max(45.0f, avail_h * 0.15f);
    float const tree_h  = std::max(80.0f, avail_h - input_h - 75.0f);

    ImGui::BeginChild("comment_tree", ImVec2{0.0f, tree_h}, false);
    draw_comment_tree(context, state);
    if (state.thread_truncated) {
        ImGui::TextDisabled("%s", tr("thread_truncated", state.current_lang));
    }
    ImGui::EndChild();
    ImGui::Separator();
    ImGui::InputTextMultiline("##reponse", state.comment_input,
                              sizeof(state.comment_input),
                              ImVec2{0.0f, input_h});
    if (ImGui::Button(tr("post_reply_btn", state.current_lang))) {
        submit_comment(context, state, 0);
    }
    ImGui::EndChild();
}

} // namespace hypercom::client
