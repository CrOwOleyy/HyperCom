# Graphical client. Only built if Dear ImGui is vendored AND glfw3 is
# available: its absence never prevents the server or the CLI client
# from being built.

hypercom_try_miniaudio(HYPERCOM_AUDIO_AVAILABLE)

add_executable(hypercom_client
    aero_decorations.cpp
    aero_theme.cpp
    bubble_reveal.cpp
    connection_guard.cpp
    draw_auth_modal.cpp
    draw_dm_panel.cpp
    draw_forum_panel.cpp
    draw_server_bar.cpp
    draw_social_panel.cpp
    draw_thread_panel.cpp
    draw_welcome_overlay.cpp
    gui_main.cpp
    gui_startup.cpp
    i18n.cpp
    intro_sequence.cpp
    ui_actions.cpp
    ui_delete_actions.cpp
    ui_dm_actions.cpp
    server_actions.cpp
    ui_report_actions.cpp
    ui_scale.cpp
    ui_social_actions.cpp
    "${CMAKE_SOURCE_DIR}/client/cli/cli_options.cpp")

# Only one of the two audio_player implementations gets compiled. So
# gui_main has no #ifdef to carry: it calls the same API either way.
if(HYPERCOM_AUDIO_AVAILABLE)
    target_sources(hypercom_client PRIVATE audio_player.cpp)
    target_link_libraries(hypercom_client PRIVATE hypercom::miniaudio)
else()
    target_sources(hypercom_client PRIVATE audio_player_silent.cpp)
endif()

target_link_libraries(hypercom_client PRIVATE
    hypercom_client_core
    hypercom::imgui)

hypercom_apply_executable_hardening(hypercom_client)
hypercom_apply_sanitizers(hypercom_client)

if(WIN32)
    # Windows subsystem: no console behind the window. In exchange,
    # stdout and stderr go nowhere, hence report_startup_failure()
    # routing startup errors through a dialog box instead.
    set_target_properties(hypercom_client PROPERTIES
        WIN32_EXECUTABLE TRUE
        LINK_FLAGS "/ENTRY:mainCRTStartup"
    )
endif()

# The welcome theme has to sit next to the binary. TARGET_FILE_DIR also
# handles MSVC's multi-configuration generators, where the executable
# lands in a Debug/ or RelWithDebInfo/ subdirectory.
if(EXISTS "${CMAKE_SOURCE_DIR}/menu.mp3")
    add_custom_command(TARGET hypercom_client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/menu.mp3"
            "$<TARGET_FILE_DIR:hypercom_client>/menu.mp3"
        COMMENT "copying menu.mp3 next to the client")
endif()
