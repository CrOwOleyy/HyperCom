#include "client/ui/draw_thread_panel.hpp"

#include "client/ui/aero_theme.hpp"
#include "client/ui/i18n.hpp"
#include "client/ui/ui_actions.hpp"
#include "client/ui/ui_delete_actions.hpp"
#include "client/ui/ui_report_actions.hpp"

#include <imgui.h>

namespace hypercom::client {
namespace {

// The button is only offered on one's own content. This isn't what
// protects other people's content -- the server checks that on its
// own side -- it only avoids showing an action that's bound to be
// refused.
[[nodiscard]] bool is_own_content(cli_context const &context,
                                  proto::wire_public_key const &author)
{
    return author == context.identity.get_public_key();
}

// Removed content keeps its row so the thread still holds together,
// but its text is empty: that emptiness is what signals it, no flag
// travels through the thread.
[[nodiscard]] bool is_removed(std::string const &body)
{
    return body.empty();
}

// Indentation depth capped for display. The server already bounds the
// tree, but the UI shouldn't rely on that bound to stay readable:
// beyond it, indentation simply stops increasing.
constexpr std::uint16_t MAX_VISUAL_DEPTH = 8;

void draw_post_header(cli_context &context, ui_state &state)
{
    ImGui::PushStyleColor(ImGuiCol_Text, AERO_INK);
    ImGui::TextWrapped("%s", state.open_post.title.c_str());
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, AERO_INK_MUTED);
    ImGui::Text("@%s", state.open_post.author_handle.c_str());
    ImGui::PopStyleColor();
    if (!is_removed(state.open_post.body)) {
        if (is_own_content(context, state.open_post.author_pubkey)) {
            ImGui::SameLine();
            if (ImGui::SmallButton(tr("delete_action", state.current_lang))) {
                delete_post(context, state, state.open_post.id);
            }
        } else {
            ImGui::SameLine();
            if (ImGui::SmallButton(tr("report_action", state.current_lang))) {
                report_post(context, state, state.open_post.id);
            }
        }
    }
    ImGui::Separator();
    ImGui::Spacing();
    if (is_removed(state.open_post.body)) {
        ImGui::PushStyleColor(ImGuiCol_Text, AERO_INK_MUTED);
        ImGui::TextUnformatted(tr("content_removed", state.current_lang));
        ImGui::PopStyleColor();
    } else {
        ImGui::TextWrapped("%s", state.open_post.body.c_str());
    }
    ImGui::Spacing();
}

void draw_comment_tree(cli_context &context, ui_state &state)
{
    for (proto::comment_record const &comment : state.comments) {
        std::uint16_t const depth =
            comment.depth > MAX_VISUAL_DEPTH ? MAX_VISUAL_DEPTH : comment.depth;
        ImGui::Indent(static_cast<float>(depth) * 16.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, AERO_INK_MUTED);
        ImGui::Text("@%s", comment.author_handle.c_str());
        ImGui::PopStyleColor();
        if (is_removed(comment.body)) {
            ImGui::PushStyleColor(ImGuiCol_Text, AERO_INK_MUTED);
            ImGui::TextUnformatted(tr("content_removed", state.current_lang));
            ImGui::PopStyleColor();
        } else {
            ImGui::TextWrapped("%s", comment.body.c_str());
        }
        ImGui::PushID(static_cast<int>(comment.id));
        if (ImGui::SmallButton(tr("reply_action", state.current_lang))) {
            submit_comment(context, state, comment.id);
        }
        if (!is_removed(comment.body)) {
            if (is_own_content(context, comment.author_pubkey)) {
                ImGui::SameLine();
                if (ImGui::SmallButton(
                        tr("delete_action", state.current_lang))) {
                    delete_comment(context, state, comment.id);
                }
            } else {
                ImGui::SameLine();
                // A comment has no reportable identifier of its own:
                // what gets reported is its author.
                if (ImGui::SmallButton(
                        tr("report_action", state.current_lang))) {
                    report_account(context, state, comment.author_pubkey);
                }
            }
        }
        ImGui::PopID();
        ImGui::Unindent(static_cast<float>(depth) * 16.0f);
        ImGui::Spacing();
    }
}

} // namespace

void draw_thread_column(cli_context &context, ui_state &state,
                        float column_width)
{
    ImGui::BeginChild("colonne_fil", ImVec2{column_width, 0.0f}, true);
    if (state.selected_post_id == 0) {
        ImGui::TextDisabled("%s", tr("post_read_empty", state.current_lang));
        ImGui::EndChild();
        return;
    }
    draw_post_header(context, state);
    draw_section_heading(tr("replies_heading", state.current_lang));

    float const avail_h = ImGui::GetContentRegionAvail().y;
    float const input_h = std::max(45.0f, avail_h * 0.15f);
    float const tree_h = std::max(80.0f, avail_h - input_h - 75.0f);

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
