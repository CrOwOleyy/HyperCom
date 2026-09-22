#include "client/ui/draw_server_bar.hpp"

#include <imgui.h>

#include "client/ui/aero_theme.hpp"
#include "client/ui/i18n.hpp"
#include "client/ui/server_actions.hpp"

namespace hypercom::client {
namespace {

constexpr float SERVER_BUTTON_HEIGHT = 34.0f;

void draw_server_button(app_state &app, server_slot const &slot,
                        std::size_t index)
{
    bool const active = app.active_slot == index;
    ImGui::PushID(static_cast<int>(index));
    if (active) {
        ImGui::PushStyleColor(ImGuiCol_Button, AERO_ACCENT);
        ImGui::PushStyleColor(ImGuiCol_Text, AERO_GLASS_STRONG);
    }
    if (ImGui::Button(slot.entry.label.c_str(),
                      ImVec2{-1.0f, SERVER_BUTTON_HEIGHT})) {
        app.active_slot = index;
    }
    if (active) {
        ImGui::PopStyleColor(2);
    }
    // Le point d'etat vit sous le bouton plutot que dedans : un bouton colore
    // par l'etat serait illisible pour qui ne distingue pas les couleurs.
    ImGui::PushStyleColor(ImGuiCol_Text, slot.view.connected ? AERO_ACCENT_TEXT
                                                             : AERO_INK_MUTED);
    ImGui::TextUnformatted(slot.view.connected ? "[ok]" : "[--]");
    ImGui::PopStyleColor();
    ImGui::PopID();
    ImGui::Spacing();
}

} // namespace

void draw_server_bar(app_state &app, server_slot_list &slots, float width)
{
    ImGui::BeginChild("barre_serveurs", ImVec2{width, 0.0f}, true);
    draw_section_heading(tr("servers_heading", app.current_lang));
    for (std::size_t index = 0; index < slots.size(); ++index) {
        draw_server_button(app, *slots[index], index);
    }
    ImGui::Separator();
    ImGui::Spacing();
    draw_section_heading(tr("servers_add_heading", app.current_lang));
    ImGui::PushStyleColor(ImGuiCol_Text, AERO_INK_MUTED);
    ImGui::TextWrapped("%s", tr("servers_add_hint", app.current_lang));
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputText("##invite", app.invite_input, sizeof(app.invite_input));
    if (ImGui::Button(tr("servers_add_btn", app.current_lang),
                      ImVec2{-1.0f, 0.0f})) {
        add_server_from_invite(app, slots);
    }
    if (!app.status_message.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, app.status_is_error
                                                 ? AERO_ALERT
                                                 : AERO_INK_MUTED);
        ImGui::TextWrapped("%s", app.status_message.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();
}

bool draw_trust_warning(app_state &app, server_slot_list &slots, float scale)
{
    if (app.active_slot >= slots.size()
        || slots[app.active_slot]->entry.trust_acknowledged) {
        return false;
    }
    server_slot const &slot = *slots[app.active_slot];
    ImGuiViewport const *const viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always,
                            ImVec2{0.5f, 0.5f});
    ImGui::SetNextWindowSize(ImVec2{560.0f * scale, 0.0f});
    ImGui::Begin("##avertissement", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                     | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    draw_section_heading(tr("trust_heading", app.current_lang));
    ImGui::TextWrapped("%s  %s", tr("trust_server", app.current_lang),
                       slot.entry.label.c_str());
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, AERO_INK);
    ImGui::TextWrapped("%s", tr("trust_body", app.current_lang));
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, AERO_INK_MUTED);
    ImGui::TextWrapped("%s", tr("trust_identity", app.current_lang));
    ImGui::PopStyleColor();
    ImGui::Spacing();
    if (ImGui::Button(tr("trust_accept", app.current_lang))) {
        acknowledge_slot_trust(app, slots, app.active_slot);
    }
    ImGui::End();
    return true;
}

} // namespace hypercom::client
