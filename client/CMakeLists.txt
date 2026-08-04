# Coeur du client, partage par le client CLI et le client graphique.
# Aucune dependance a ImGui ici : le coeur doit rester testable et compilable
# meme la ou aucune bibliotheque graphique n'est installee.
add_library(hypercom_client_core STATIC
    dm/dm_courier.cpp
    keystore/identity_store.cpp
    net/client_session.cpp
    net/server_connection.cpp
    net/tcp_client_socket.cpp)

target_include_directories(hypercom_client_core PUBLIC "${CMAKE_SOURCE_DIR}")
target_link_libraries(hypercom_client_core PUBLIC hypercom_common)
if(WIN32)
    target_link_libraries(hypercom_client_core PUBLIC ws2_32)
endif()
hypercom_apply_strict_options(hypercom_client_core)
hypercom_apply_sanitizers(hypercom_client_core)

# Client en ligne de commande : sert a valider le protocole et la crypto sans
# dependre du rendu (BRIEF.md 14, question 3).
add_executable(hypercom_cli
    cli/cli_content_commands.cpp
    cli/cli_dm_commands.cpp
    cli/cli_forum_commands.cpp
    cli/cli_options.cpp
    cli/cli_social_commands.cpp
    cli/cli_top8_commands.cpp
    cli_main.cpp)
target_link_libraries(hypercom_cli PRIVATE hypercom_client_core)
hypercom_apply_executable_hardening(hypercom_cli)
hypercom_apply_sanitizers(hypercom_cli)

if(HYPERCOM_BUILD_GUI)
    hypercom_try_imgui(HYPERCOM_IMGUI_AVAILABLE)
    if(HYPERCOM_IMGUI_AVAILABLE)
        add_subdirectory(ui)
    endif()
endif()
