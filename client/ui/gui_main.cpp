#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>
#include <string>

#include "client/cli/cli_options.hpp"
#include "client/ui/aero_decorations.hpp"
#include "client/ui/aero_theme.hpp"
#include "client/ui/audio_player.hpp"
#include "client/ui/bubble_reveal.hpp"
#include "client/ui/draw_auth_modal.hpp"
#include "client/ui/draw_dm_panel.hpp"
#include "client/ui/draw_forum_panel.hpp"
#include "client/ui/draw_thread_panel.hpp"
#include "client/ui/draw_welcome_overlay.hpp"
#include "client/ui/gui_startup.hpp"
#include "client/ui/intro_sequence.hpp"
#include "client/ui/ui_actions.hpp"
#include "client/ui/ui_scale.hpp"
#include "common/crypto/sodium_runtime.hpp"
#include "common/util/hex_codec.hpp"

namespace {

using namespace hypercom;

constexpr int WINDOW_MIN_WIDTH = 900;
constexpr int WINDOW_MIN_HEIGHT = 600;
constexpr char const *THEME_FILE_NAME = "menu.mp3";
constexpr int REVEAL_BLOCK_COUNT = 3;

void draw_top_bar(client::ui_state &state,
                  client::ui_scale_state &scale_state)
{
    client::draw_status_dot(state.connected,
                            state.connected
                                ? client::tr("status_encrypted", state.current_lang)
                                : client::tr("status_offline", state.current_lang));
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    ImGui::TextDisabled("%s %s", client::tr("status_server", state.current_lang),
                        state.server_key_hex.substr(0, 16).c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    ImGui::TextDisabled("%s %s", client::tr("status_identity", state.current_lang),
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

// Les trois colonnes remontent l'une apres l'autre pendant la phase de
// revelation. Le decalage vertical doit etre applique APRES SameLine, sinon
// ImGui le remet a la ligne de base et l'effet disparait.
void draw_columns(client::cli_context &context, client::ui_state &state,
                  float scale, client::intro_state const &intro)
{
    ImGuiViewport const *const viewport = ImGui::GetMainViewport();
    ImGuiStyle const &style = ImGui::GetStyle();
    float const usable = viewport->WorkSize.x - style.WindowPadding.x * 2.0f
                         - style.ItemSpacing.x * 2.0f;
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

void draw_application_frame(client::cli_context &context,
                            client::ui_state &state,
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
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGuiWindowFlags const flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("hypercom", nullptr, flags);
    draw_top_bar(state, scale_state);
    static_cast<void>(client::handle_zoom_input(scale_state));
    if (state.registered) {
        draw_columns(context, state, scale, intro);
    } else {
        client::draw_auth_modal(context, state, scale);
    }
    ImGui::End();
}

// Une fenetre de 1280x800 codee en dur occupe un quart d'un ecran 4K. On part
// donc d'une fraction de la surface reellement disponible.
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
    height = std::max(WINDOW_MIN_HEIGHT,
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

// Reconstruire l'atlas de police detruit la texture du backend OpenGL.
// L'operation ne peut donc avoir lieu qu'entre deux images.
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

void run_render_loop(GLFWwindow *window, client::cli_context &context,
                     client::ui_state &state,
                     client::ui_scale_state &scale_state,
                     client::intro_state &intro, client::audio_player &audio)
{
    while (glfwWindowShouldClose(window) == 0) {
        glfwPollEvents();
        refresh_scaling_if_needed(scale_state);
        if (state.intro_requested) {
            state.intro_requested = false;
            std::string audio_failure;
            // L'echec audio n'interrompt rien : la sequence se deroule sur
            // l'horloge, avec une duree de repli.
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
        draw_application_frame(context, state, scale_state, intro);
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
    if (!client::parse_cli_options(argc, argv, options, failure, false)) {
        client::report_startup_failure(failure);
        return 2;
    }
    crypto::identity_keypair identity;
    crypto::x25519_public_key server_key{};
    if (!client::prepare_session(options, identity, server_key, failure)) {
        client::report_startup_failure(failure);
        return 3;
    }
    client::server_connection connection{server_key};
    if (!connection.open_session(options.host, options.port, failure)) {
        client::report_startup_failure("Connexion impossible : " + failure);
        return 4;
    }
    client::client_session session{connection, identity};
    if (!session.authenticate(failure)) {
        client::report_startup_failure("Authentification refusee : " + failure);
        return 5;
    }
    bool is_registered = !session.needs_registration();
    bool account_just_created = false;
    if (!is_registered && !options.command.empty()) {
        std::string registration_failure;
        if (session.register_handle(options.command, registration_failure)) {
            is_registered = true;
            account_just_created = true;
        }
    }
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
              << client::BASE_FONT_SIZE
                     * client::compute_effective_scale(scale_state)
              << " px)\n"
              << std::flush;
    client::ui_state state;
    state.connected = connection.is_open();
    state.server_host = options.host;
    state.server_port = options.port;
    state.registered = is_registered;
    // Creer son compte par argument de ligne de commande reste une creation de
    // compte : meme accueil que par la fenetre d'inscription. --replay-intro
    // force la sequence sur un compte existant, pour pouvoir la regler.
    state.intro_requested = account_just_created || options.replay_intro;
    if (is_registered) {
        state.handle = session.get_handle();
    }
    util::encode_hex(identity.get_public_key(), state.identity_hex);
    state.server_key_hex = options.server_key_hex;
    client::cli_context context{connection, identity, session};
    if (is_registered) {
        client::refresh_forum_list(context, state);
    }
    client::intro_state intro;
    client::audio_player audio;
    run_render_loop(window, context, state, scale_state, intro, audio);
    audio.stop_track();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
