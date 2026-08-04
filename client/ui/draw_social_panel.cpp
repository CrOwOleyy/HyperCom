#include "client/ui/draw_social_panel.hpp"

#include <algorithm>
#include <imgui.h>

#include "client/ui/aero_theme.hpp"
#include "client/ui/i18n.hpp"
#include "client/ui/ui_social_actions.hpp"

namespace hypercom::client {
namespace {

[[nodiscard]] proto::friend_record const *find_slot_detail(
    proto::wire_public_key const &slot,
    std::vector<proto::friend_record> const &details)
{
    if (slot == proto::wire_public_key{}) {
        return nullptr;
    }
    for (proto::friend_record const &record : details) {
        if (record.pubkey == slot) {
            return &record;
        }
    }
    return nullptr;
}

// removable_owner non nul : chaque case occupee gagne un bouton "retirer" qui
// vide la case correspondante dans son propre top 8. Nul : affichage seul,
// pour le top 8 d'un profil consulte.
void draw_top8_grid(
    ui_state *removable_owner,
    std::array<proto::wire_public_key, proto::TOP8_SLOT_COUNT> const &slots,
    std::vector<proto::friend_record> const &details)
{
    for (std::size_t index = 0; index < slots.size(); ++index) {
        proto::friend_record const *const detail =
            find_slot_detail(slots[index], details);
        ImGui::PushID(static_cast<int>(index));
        if (detail != nullptr) {
            ImGui::Text("%d. @%s", static_cast<int>(index + 1),
                       detail->handle.c_str());
            if (removable_owner != nullptr) {
                ImGui::SameLine();
                if (ImGui::SmallButton("x")) {
                    removable_owner->own_top8_slots[index] =
                        proto::wire_public_key{};
                }
            }
        } else {
            ImGui::TextDisabled("%d. --", static_cast<int>(index + 1));
        }
        ImGui::PopID();
    }
}

void draw_friends_panel(cli_context &context, ui_state &state, float height)
{
    draw_section_heading(tr("friends_heading", state.current_lang));
    if (ImGui::Button(tr("forum_btn_refresh", state.current_lang))) {
        refresh_friend_list(context, state);
    }
    ImGui::BeginChild("friends_list", ImVec2{0.0f, height}, true);
    for (std::size_t index = 0; index < state.friends.size(); ++index) {
        proto::friend_record const &entry = state.friends[index];
        ImGui::PushID(static_cast<int>(index));
        ImGui::TextUnformatted(("@" + entry.handle).c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton(tr("social_view_btn", state.current_lang))) {
            view_profile(context, state, entry.pubkey);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton(tr("social_favorite_btn", state.current_lang))) {
            for (proto::wire_public_key &slot : state.own_top8_slots) {
                if (slot == proto::wire_public_key{}) {
                    slot = entry.pubkey;
                    break;
                }
            }
        }
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::InputText(tr("friends_add", state.current_lang),
                     state.friend_add_input, sizeof(state.friend_add_input));
    ImGui::SameLine();
    if (ImGui::SmallButton(tr("social_add_btn", state.current_lang))) {
        add_friend(context, state);
    }
}

void draw_viewed_profile_panel(ui_state const &state, float height)
{
    draw_section_heading(tr("viewed_profile_heading", state.current_lang));
    if (state.viewed_profile.handle.empty()) {
        ImGui::TextDisabled("%s",
                            tr("viewed_profile_empty", state.current_lang));
        return;
    }
    ImGui::Text("@%s  (%s)", state.viewed_profile.handle.c_str(),
               state.viewed_profile.display_name.c_str());
    ImGui::PushStyleColor(ImGuiCol_Text, AERO_INK_MUTED);
    ImGui::TextWrapped("%s", state.viewed_profile.bio.c_str());
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::BeginChild("viewed_top8", ImVec2{0.0f, height}, true);
    draw_top8_grid(nullptr, state.viewed_top8_slots, state.viewed_top8_details);
    ImGui::EndChild();
}

void draw_own_top8_editor(cli_context &context, ui_state &state, float height)
{
    draw_section_heading(tr("top8_heading", state.current_lang));
    ImGui::BeginChild("own_top8", ImVec2{0.0f, height}, true);
    draw_top8_grid(&state, state.own_top8_slots, state.own_top8_details);
    ImGui::EndChild();
    if (ImGui::Button(tr("social_save_btn", state.current_lang))) {
        submit_own_top8(context, state);
    }
}

} // namespace

void draw_social_panel(cli_context &context, ui_state &state, float height)
{
    float const friends_h = std::max(90.0f, height * 0.32f);
    float const profile_h = std::max(70.0f, height * 0.22f);
    float const editor_h = std::max(80.0f, height * 0.26f);
    draw_friends_panel(context, state, friends_h);
    ImGui::Spacing();
    draw_viewed_profile_panel(state, profile_h);
    ImGui::Spacing();
    draw_own_top8_editor(context, state, editor_h);
}

} // namespace hypercom::client
