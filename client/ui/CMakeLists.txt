# Client graphique. Construit uniquement si Dear ImGui est vendorise ET si
# glfw3 est disponible : son absence n'empeche jamais le serveur ni le client
# CLI d'etre construits.
add_executable(hypercom_client
    draw_auth_modal.cpp
    draw_dm_panel.cpp
    draw_forum_panel.cpp
    draw_thread_panel.cpp
    gui_main.cpp
    pale_green_theme.cpp
    ui_actions.cpp
    ui_dm_actions.cpp
    ui_scale.cpp
    "${CMAKE_SOURCE_DIR}/client/cli/cli_options.cpp")

target_link_libraries(hypercom_client PRIVATE
    hypercom_client_core
    hypercom::imgui)

hypercom_apply_executable_hardening(hypercom_client)
hypercom_apply_sanitizers(hypercom_client)
