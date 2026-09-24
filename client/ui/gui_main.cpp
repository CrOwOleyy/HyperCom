#include "client/cli/cli_options.hpp"
#include "client/keystore/server_registry.hpp"
#include "client/ui/aero_decorations.hpp"
#include "client/ui/aero_theme.hpp"
#include "client/ui/app_state.hpp"
#include "client/ui/audio_player.hpp"
#include "client/ui/bubble_reveal.hpp"
#include "client/ui/draw_auth_modal.hpp"
#include "client/ui/draw_dm_panel.hpp"
#include "client/ui/draw_forum_panel.hpp"
#include "client/ui/draw_server_bar.hpp"
#include "client/ui/draw_thread_panel.hpp"
#include "client/ui/draw_welcome_overlay.hpp"
#include "client/ui/gui_startup.hpp"
#include "client/ui/intro_sequence.hpp"
#include "client/ui/server_actions.hpp"
#include "client/ui/server_slot.hpp"
#include "client/ui/ui_actions.hpp"
#include "client/ui/ui_scale.hpp"
#include "common/crypto/sodium_runtime.hpp"
#include "common/util/hex_codec.hpp"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <string>

namespace {

using namespace hypercom;

constexpr int WINDOW_MIN_WIDTH = 900;
constexpr int WINDOW_MIN_HEIGHT = 600;
constexpr char const *THEME_FILE_NAME = "menu.mp3";
constexpr int REVEAL_BLOCK_COUNT = 3;

void draw_top_bar(client::ui_state &state, client::ui_scale_state &scale_state)
{
    client::draw_status_dot(
        state.connected,
        state.connected ? client::tr("status_encrypted", state.current_lang)
                        : client::tr("status_offline", state.current_lang));
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    ImGui::TextDisabled("%s %s",
                        client::tr("status_server", state.current_lang),
                        state.server_key_hex.substr(0, 16).c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    ImGui::TextDisabled("%s %s",
                        client::tr("status_identity", state.current_lang),
                        state.identity_hex.substr(0, 16).c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    client::draw_zoom_controls(scale_state, state);
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    if (state.current_lang == client::language::french) {
        ImGui::TextDisabled("FR");
    } else {
        if (ImGui::SmallButton("FR")) {
            state.current_lang = client::language::french;
        }
    }
    ImGui::SameLine();
    if (state.current_lang == client::language::english) {
        ImGui::TextDisabled("EN");
    } else {
        if (ImGui::SmallButton("EN")) {
            state.current_lang = client::language::english;
        }
    }
    if (!state.status_message.empty()) {
        ImVec4 const tint =
            state.status_is_error ? client::AERO_ALERT : client::AERO_INK_MUTED;
        ImGui::PushStyleColor(ImGuiCol_Text, tint);
        ImGui::TextWrapped("%s", state.status_message.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::Separator();
}

// The three columns rise one after another during the reveal phase.
// The vertical offset must be applied AFTER SameLine, otherwise ImGui
// resets it to the baseline and the effect disappears.
void draw_columns(client::cli_context &context, client::ui_state &state,
                  float scale, client::intro_state const &intro)
{
    ImGuiViewport const *const viewport = ImGui::GetMainViewport();
    ImGuiStyle const &style = ImGui::GetStyle();
    float const usable = viewport->WorkSize.x - style.WindowPadding.x * 2.0f -
                         style.ItemSpacing.x * 2.0f;
    float const forum_width = std::max(300.0f * scale, usable * 0.26f);
    float const side_width = std::max(330.0f * scale, usable * 0.28f);
    float const thread_width = usable - forum_width - side_width;
    client::begin_bubble_reveal(
        client::compute_element_reveal(intro, 0, REVEAL_BLOCK_COUNT), scale);
    client::draw_forum_column(context, state, forum_width);
    client::end_bubble_reveal();
    ImGui::SameLine();
    client::begin_bubble_reveal(
        client::compute_element_reveal(intro, 1, REVEAL_BLOCK_COUNT), scale);
    client::draw_thread_column(context, state, thread_width);
    client::end_bubble_reveal();
    ImGui::SameLine();
    client::begin_bubble_reveal(
        client::compute_element_reveal(intro, 2, REVEAL_BLOCK_COUNT), scale);
    client::draw_side_column(context, state, side_width);
    client::end_bubble_reveal();
}

// The registry if it exists, otherwise a single server built from
// --host, --port and --server-key. This fallback keeps existing
// scripts and shortcuts working, since they don't know about the
// registry yet.
[[nodiscard]] bool load_server_entries(client::cli_options const &options,
                                       std::string_view passphrase,
                                       std::vector<client::server_entry> &out,
                                       std::string &error_out)
{
    client::server_registry registry{options.registry_path};
    if (registry.has_stored_registry()) {
        return registry.load(passphrase, out, error_out);
    }
    if (options.server_key_hex.size() != 64) {
        error_out = "Aucun serveur enregistre, et --server-key absent.\n\n"
                    "Ajoutez un serveur avec hypercom_cli server-add, ou "
                    "passez --server-key.";
        return false;
    }
    client::server_entry entry;
    entry.label = options.host;
    entry.endpoint = {options.host, options.port, options.socks5_host,
                      options.socks5_port};
    std::vector<std::uint8_t> decoded;
    if (!util::decode_hex(options.server_key_hex, decoded) ||
        decoded.size() != entry.server_key.size()) {
        error_out = "--server-key doit faire 64 caracteres hexadecimaux.";
        return false;
    }
    std::copy(decoded.begin(), decoded.end(), entry.server_key.begin());
    entry.source = client::identity_source::imported;
    entry.imported_identity_path = options.identity_path;
    // Direct mode assumes a server already chosen knowingly: showing
    // the warning again on every launch wouldn't teach anything new.
    entry.trust_acknowledged = true;
    out.push_back(std::move(entry));
    return true;
}

// The displayed server is connected on demand, never before its
// warning has been acknowledged. The other slots stay inactive until
// switched to: opening N Noise sessions -- and N Tor circuits -- at
// startup would cost several seconds per server.
void service_active_slot(client::app_state &app,
                         client::server_slot_list &slots)
{
    if (app.active_slot >= slots.size()) {
        return;
    }
    client::server_slot &slot = *slots[app.active_slot];
    slot.view.current_lang = app.current_lang;
    // The welcome flag is raised by draw_auth_modal on the slot that
    // just created an account, but the sequence occupies the whole
    // window: it plays out at the application level.
    if (slot.view.intro_requested) {
        slot.view.intro_requested = false;
        app.intro_requested = true;
    }
    if (!slot.entry.trust_acknowledged || slot.connection->is_open()) {
        return;
    }
    std::string failure;
    if (!client::connect_slot(slot, failure)) {
        slot.view.status_message = failure;
        slot.view.status_is_error = true;
        return;
    }
    if (slot.view.registered && slot.view.forums.empty()) {
        client::cli_context context = client::make_context(slot);
        client::refresh_forum_list(context, slot.view);
    }
}

void draw_application_frame(client::app_state &app,
                            client::server_slot_list &slots,
                            client::ui_scale_state &scale_state,
                            client::intro_state const &intro)
{
    ImGuiViewport const *const viewport = ImGui::GetMainViewport();
    client::draw_aero_backdrop(ImGui::GetBackgroundDrawList(),
                               viewport->WorkPos, viewport->WorkSize,
                               static_cast<float>(ImGui::GetTime()));
    float const scale = client::compute_effective_scale(scale_state);
    if (client::get_intro_phase(intro) == client::intro_phase::welcome) {
        client::draw_welcome_overlay(intro, scale);
        return;
    }
    service_active_slot(app, slots);
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGuiWindowFlags const flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("hypercom", nullptr, flags);
    float const bar_width =
        std::max(150.0f * scale, viewport->WorkSize.x * 0.11f);
    client::draw_server_bar(app, slots, bar_width);
    ImGui::SameLine();
    ImGui::BeginChild("zone_serveur", ImVec2{0.0f, 0.0f}, false);
    if (app.active_slot < slots.size()) {
        client::server_slot &slot = *slots[app.active_slot];
        draw_top_bar(slot.view, scale_state);
        static_cast<void>(client::handle_zoom_input(scale_state));
        app.current_lang = slot.view.current_lang;
        client::cli_context context = client::make_context(slot);
        if (slot.view.registered) {
            draw_columns(context, slot.view, scale, intro);
        } else if (slot.entry.trust_acknowledged) {
            client::draw_auth_modal(context, slot.view, scale);
        }
    }
    ImGui::EndChild();
    ImGui::End();
    static_cast<void>(client::draw_trust_warning(app, slots, scale));
}

// A hardcoded 1280x800 window occupies a quarter of a 4K screen. So we
// start from a fraction of the actually available area instead.
void compute_initial_window_size(int &width, int &height)
{
    width = WINDOW_MIN_WIDTH;
    height = WINDOW_MIN_HEIGHT;
    GLFWmonitor *const monitor = glfwGetPrimaryMonitor();
    if (monitor == nullptr) {
        return;
    }
    int origin_x = 0;
    int origin_y = 0;
    int work_width = 0;
    int work_height = 0;
    glfwGetMonitorWorkarea(monitor, &origin_x, &origin_y, &work_width,
                           &work_height);
    if (work_width <= 0 || work_height <= 0) {
        return;
    }
    width = std::max(WINDOW_MIN_WIDTH,
                     static_cast<int>(static_cast<float>(work_width) * 0.82f));
    height =
        std::max(WINDOW_MIN_HEIGHT,
                 static_cast<int>(static_cast<float>(work_height) * 0.85f));
}

[[nodiscard]] GLFWwindow *create_window()
{
    if (glfwInit() == 0) {
        return nullptr;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    int width = WINDOW_MIN_WIDTH;
    int height = WINDOW_MIN_HEIGHT;
    compute_initial_window_size(width, height);
    GLFWwindow *const window =
        glfwCreateWindow(width, height, "hypercom", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return nullptr;
    }
    glfwSetWindowSizeLimits(window, WINDOW_MIN_WIDTH, WINDOW_MIN_HEIGHT,
                            GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    return window;
}

// Rebuilding the font atlas destroys the OpenGL backend's texture.
// The operation can therefore only happen between two frames.
void refresh_scaling_if_needed(client::ui_scale_state &scale_state)
{
    if (!scale_state.font_rebuild_needed) {
        return;
    }
    client::rebuild_scaled_font(scale_state);
    ImGui_ImplOpenGL3_DestroyFontsTexture();
    static_cast<void>(ImGui_ImplOpenGL3_CreateFontsTexture());
    client::apply_aero_theme(ImGui::GetStyle(),
                             client::compute_effective_scale(scale_state));
}

void run_render_loop(GLFWwindow *window, client::app_state &app,
                     client::server_slot_list &slots,
                     client::ui_scale_state &scale_state,
                     client::intro_state &intro, client::audio_player &audio)
{
    while (glfwWindowShouldClose(window) == 0) {
        glfwPollEvents();
        refresh_scaling_if_needed(scale_state);
        if (app.intro_requested) {
            app.intro_requested = false;
            std::string audio_failure;
            // An audio failure interrupts nothing: the sequence runs
            // on the clock, with a fallback duration.
            bool const playing =
                audio.start_track(THEME_FILE_NAME, audio_failure);
            client::begin_intro(intro, audio.get_track_length_seconds());
            std::cout << "intro : "
                      << (playing ? std::string{"musique"} : audio_failure)
                      << ", duree retenue " << intro.total_seconds << " s\n"
                      << std::flush;
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        client::advance_intro(intro,
                              static_cast<double>(ImGui::GetIO().DeltaTime));
        draw_application_frame(app, slots, scale_state, intro);
        ImGui::Render();
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(client::AERO_SKY_TOP.x, client::AERO_SKY_TOP.y,
                     client::AERO_SKY_TOP.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}

} // namespace

int main(int argc, char **argv)
{
    if (!crypto::initialize_sodium()) {
        client::report_startup_failure("libsodium n'a pas pu s'initialiser.");
        return 1;
    }
    client::cli_options options;
    std::string failure;
    if (!client::parse_cli_options(argc, argv, options, failure, false,
                                   false)) {
        client::report_startup_failure(failure);
        return 2;
    }
    client::app_state app;
    app.registry_path = options.registry_path;
    app.master_seed_path = options.master_seed_path;
    if (!client::read_passphrase(app.passphrase, failure)) {
        client::report_startup_failure(failure);
        return 3;
    }
    std::vector<client::server_entry> entries;
    if (!load_server_entries(options, app.passphrase, entries, failure)) {
        client::report_startup_failure(failure);
        return 4;
    }
    client::server_slot_list slots;
    for (client::server_entry const &entry : entries) {
        std::unique_ptr<client::server_slot> slot;
        if (!client::build_slot(entry, app, slot, failure)) {
            client::report_startup_failure(failure);
            return 5;
        }
        slots.push_back(std::move(slot));
    }
    app.intro_requested = options.replay_intro;
    GLFWwindow *const window = create_window();
    if (window == nullptr) {
        client::report_startup_failure(
            "Creation de la fenetre impossible : pilote OpenGL 3.3 absent ?");
        return 7;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    client::ui_scale_state scale_state;
    client::detect_display_scale(window, scale_state);
    std::cout << "affichage : echelle "
              << client::compute_effective_scale(scale_state) << " (police "
              << client::BASE_FONT_SIZE *
                     client::compute_effective_scale(scale_state)
              << " px)\n"
              << std::flush;
    client::intro_state intro;
    client::audio_player audio;
    run_render_loop(window, app, slots, scale_state, intro, audio);
    audio.stop_track();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
