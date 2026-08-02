#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>
#include <string>

#include "client/cli/cli_options.hpp"
#include "client/keystore/identity_store.hpp"
#include "client/ui/draw_auth_modal.hpp"
#include "client/ui/draw_dm_panel.hpp"
#include "client/ui/draw_forum_panel.hpp"
#include "client/ui/draw_thread_panel.hpp"
#include "client/ui/pale_green_theme.hpp"
#include "client/ui/ui_actions.hpp"
#include "client/ui/ui_scale.hpp"
#include "common/crypto/sodium_runtime.hpp"
#include "common/util/hex_codec.hpp"

namespace {

using namespace hypercom;

constexpr int WINDOW_MIN_WIDTH = 900;
constexpr int WINDOW_MIN_HEIGHT = 600;

void draw_top_bar(client::ui_state const &state,
                  client::ui_scale_state &scale_state)
{
    client::draw_status_dot(state.connected,
                            state.connected
                                ? "Canal chiffré (Noise_NK) et authentifié"
                                : "Hors ligne");
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    ImGui::TextDisabled("Serveur %s",
                        state.server_key_hex.substr(0, 16).c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    client::draw_zoom_controls(scale_state);
    if (!state.status_message.empty()) {
        ImVec4 const tint = state.status_is_error ? client::PALE_GREEN_ALERT
                                                  : client::PALE_GREEN_INK_MUTED;
        ImGui::PushStyleColor(ImGuiCol_Text, tint);
        ImGui::TextWrapped("%s", state.status_message.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::Separator();
}

void draw_application_frame(client::cli_context &context,
                            client::ui_state &state,
                            client::ui_scale_state &scale_state)
{
    ImGuiViewport const *const viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGuiWindowFlags const flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("hypercom", nullptr, flags);
    draw_top_bar(state, scale_state);
    static_cast<void>(client::handle_zoom_input(scale_state));
    float const scale = client::compute_effective_scale(scale_state);

    if (!state.registered) {
        client::draw_auth_modal(context, state, scale);
        ImGui::End();
        return;
    }

    ImGuiStyle const &style = ImGui::GetStyle();
    float const usable = viewport->WorkSize.x - style.WindowPadding.x * 2.0f
                         - style.ItemSpacing.x * 2.0f;
    float const forum_width = std::max(300.0f * scale, usable * 0.26f);
    float const side_width = std::max(330.0f * scale, usable * 0.28f);
    float const thread_width = usable - forum_width - side_width;
    client::draw_forum_column(context, state, forum_width);
    ImGui::SameLine();
    client::draw_thread_column(context, state, thread_width);
    ImGui::SameLine();
    client::draw_side_column(context, state, side_width);
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

// Reconstruire l'atlas de police detruit la texture que le backend OpenGL
// utilise. L'operation ne peut donc avoir lieu qu'entre deux images, jamais
// entre NewFrame et Render.
void refresh_scaling_if_needed(client::ui_scale_state &scale_state)
{
    if (!scale_state.font_rebuild_needed) {
        return;
    }
    client::rebuild_scaled_font(scale_state);
    ImGui_ImplOpenGL3_DestroyFontsTexture();
    static_cast<void>(ImGui_ImplOpenGL3_CreateFontsTexture());
    client::apply_pale_green_theme(
        ImGui::GetStyle(), client::compute_effective_scale(scale_state));
}

void run_render_loop(GLFWwindow *window, client::cli_context &context,
                     client::ui_state &state,
                     client::ui_scale_state &scale_state)
{
    while (glfwWindowShouldClose(window) == 0) {
        glfwPollEvents();
        refresh_scaling_if_needed(scale_state);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        draw_application_frame(context, state, scale_state);
        ImGui::Render();
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(client::PALE_GREEN_CANVAS.x, client::PALE_GREEN_CANVAS.y,
                     client::PALE_GREEN_CANVAS.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}

[[nodiscard]] bool connect_and_authenticate(
    client::cli_options const &options, crypto::identity_keypair &identity,
    crypto::x25519_public_key &server_key, std::string &error_out)
{
    std::vector<std::uint8_t> decoded;
    if (!util::decode_hex(options.server_key_hex, decoded)
        || decoded.size() != server_key.size()) {
        error_out = "--server-key doit faire 64 caracteres hexadecimaux";
        return false;
    }
    std::copy(decoded.begin(), decoded.end(), server_key.begin());
    client::identity_store store{options.identity_path};
    std::string passphrase;
    if (!client::read_passphrase(passphrase, error_out)) {
        return false;
    }
    if (store.has_stored_identity()) {
        return store.unlock_identity(passphrase, identity, error_out);
    }
    return store.create_identity(passphrase, identity, error_out);
}

} // namespace

int main(int argc, char **argv)
{
    if (!crypto::initialize_sodium()) {
        std::cerr << "libsodium n'a pas pu s'initialiser\n";
        return 1;
    }
    client::cli_options options;
    std::string failure;
    if (!client::parse_cli_options(argc, argv, options, failure, false)) {
        std::cerr << failure << '\n';
        client::print_cli_usage();
        return 1;
    }
    crypto::identity_keypair identity;
    crypto::x25519_public_key server_key{};
    if (!connect_and_authenticate(options, identity, server_key, failure)) {
        std::cerr << failure << '\n';
        return 3;
    }
    client::server_connection connection{server_key};
    if (!connection.open_session(options.host, options.port, failure)) {
        std::cerr << failure << '\n';
        return 4;
    }
    client::client_session session{connection, identity};
    if (!session.authenticate(failure)) {
        std::cerr << failure << '\n';
        return 5;
    }
    bool is_registered = !session.needs_registration();
    if (!is_registered && !options.command.empty()) {
        std::string reg_failure;
        if (session.register_handle(options.command, reg_failure)) {
            is_registered = true;
        }
    }
    GLFWwindow *const window = create_window();
    if (window == nullptr) {
        std::cerr << "creation de la fenetre impossible\n";
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
              << "            Ctrl + / Ctrl - pour ajuster, Ctrl 0 pour "
                 "revenir a 100 %\n"
              << std::flush;
    client::ui_state state;
    state.connected = true;
    state.registered = is_registered;
    if (is_registered) {
        state.handle = session.get_handle();
    }
    util::encode_hex(identity.get_public_key(), state.identity_hex);
    state.server_key_hex = options.server_key_hex;
    client::cli_context context{connection, identity, session};
    if (is_registered) {
        client::refresh_forum_list(context, state);
    }
    run_render_loop(window, context, state, scale_state);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
