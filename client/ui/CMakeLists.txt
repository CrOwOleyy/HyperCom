# Client graphique. Construit uniquement si Dear ImGui est vendorise ET si
# glfw3 est disponible : son absence n'empeche jamais le serveur ni le client
# CLI d'etre construits.

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
    ui_scale.cpp
    ui_social_actions.cpp
    "${CMAKE_SOURCE_DIR}/client/cli/cli_options.cpp")

# Une seule des deux implementations d'audio_player est compilee. gui_main n'a
# donc aucun #ifdef a porter : il appelle la meme API dans les deux cas.
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
    # Sous-systeme Windows : pas de console derriere la fenetre. En contrepartie
    # stdout et stderr n'aboutissent nulle part, d'ou report_startup_failure()
    # qui passe par une boite de dialogue pour les erreurs de demarrage.
    set_target_properties(hypercom_client PROPERTIES
        WIN32_EXECUTABLE TRUE
        LINK_FLAGS "/ENTRY:mainCRTStartup"
    )
endif()

# Le theme d'accueil doit se trouver a cote du binaire. TARGET_FILE_DIR gere
# aussi les generateurs multi-configuration de MSVC, ou l'executable atterrit
# dans un sous-repertoire Debug/ ou RelWithDebInfo/.
if(EXISTS "${CMAKE_SOURCE_DIR}/menu.mp3")
    add_custom_command(TARGET hypercom_client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/menu.mp3"
            "$<TARGET_FILE_DIR:hypercom_client>/menu.mp3"
        COMMENT "copie de menu.mp3 a cote du client")
endif()
