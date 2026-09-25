# Client core, shared by the CLI client and the graphical client.
# No dependency on ImGui here: the core has to stay testable and
# compilable even where no graphics library is installed.
add_library(hypercom_client_core STATIC
    cli/invite_link.cpp
    dm/dm_courier.cpp
    keystore/identity_store.cpp
    keystore/master_seed_store.cpp
    keystore/sealed_file.cpp
    keystore/server_identity.cpp
    keystore/server_registry.cpp
    net/client_session.cpp
    net/server_connection.cpp
    net/socks5_connector.cpp
    net/tcp_client_socket.cpp)

target_include_directories(hypercom_client_core PUBLIC "${CMAKE_SOURCE_DIR}")
target_link_libraries(hypercom_client_core PUBLIC hypercom_common)
if(WIN32)
    target_link_libraries(hypercom_client_core PUBLIC ws2_32)
endif()
hypercom_apply_strict_options(hypercom_client_core)
hypercom_apply_sanitizers(hypercom_client_core)

# Command-line client: used to validate the protocol and the crypto
# without depending on rendering.
add_executable(hypercom_cli
    cli/cli_content_commands.cpp
    cli/cli_content_delete_commands.cpp
    cli/cli_dm_commands.cpp
    cli/cli_forum_commands.cpp
    cli/cli_options.cpp
    cli/cli_report_commands.cpp
    cli/cli_server_commands.cpp
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
