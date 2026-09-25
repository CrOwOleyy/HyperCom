cmake_minimum_required(VERSION 3.20)

project(hypercom
    VERSION 1.0.0
    DESCRIPTION "Unmoderated, end-to-end encrypted social network"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(HYPERCOM_BUILD_SERVER "Build the server (Linux only)"       ON)
option(HYPERCOM_BUILD_GUI    "Build the ImGui graphical client"    ON)
option(HYPERCOM_BUILD_TESTS  "Build the unit tests"                ON)
option(HYPERCOM_BUILD_FUZZ   "Build the libFuzzer harnesses"       OFF)

include(hypercom_compile_options)
include(hypercom_sanitizers)
include(hypercom_dependencies)

hypercom_require_libsodium()
hypercom_require_sqlite3()

add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(tools)

if(HYPERCOM_BUILD_SERVER)
    add_subdirectory(server)
endif()

if(HYPERCOM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

include(GNUInstallDirs)

install(TARGETS hypercom_cli DESTINATION ${CMAKE_INSTALL_BINDIR})

if(TARGET hypercom_client)
    install(TARGETS hypercom_client DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(HYPERCOM_BUILD_SERVER)
    install(TARGETS hypercom_server DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

set(CPACK_PACKAGE_NAME "hypercom")
set(CPACK_PACKAGE_VENDOR "Hypercom")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "End-to-end encrypted social network")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "Hypercom Maintainers <contact@hypercom.local>")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Hypercom Maintainers <contact@hypercom.local>")
set(CPACK_DEBIAN_PACKAGE_SECTION "net")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsodium23 | libsodium-dev, libsqlite3-0 | libsqlite3-dev")

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
endif()

include(CPack)

message(STATUS "hypercom: ${CMAKE_BUILD_TYPE} configuration for ${CMAKE_SYSTEM_NAME}")
