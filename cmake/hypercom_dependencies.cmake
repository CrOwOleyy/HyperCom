# Resolution of the only three dependencies allowed by the project's
# norm: libsodium, SQLite and Dear ImGui.
#
# Three possible sources, in this priority order:
#   1. third_party/<name>         (vendored, recommended for production)
#   2. system packages            (pkg-config or find_package)
#   3. explicit failure with the exact command to run
#
# No network fetch is ever triggered by CMake: downloading is a
# deliberate action, done through scripts/fetch_third_party.sh.

set(HYPERCOM_THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party")

function(hypercom_fail_missing_dependency name hint)
    message(FATAL_ERROR
        "hypercom: dependency '${name}' not found.\n"
        "  Option 1 (vendored): ./scripts/fetch_third_party.sh\n"
        "  Option 2 (system)  : ${hint}")
endfunction()

function(hypercom_require_libsodium)
    if(TARGET hypercom::sodium)
        return()
    endif()
    add_library(hypercom_sodium INTERFACE)
    add_library(hypercom::sodium ALIAS hypercom_sodium)
    if(EXISTS "${HYPERCOM_THIRD_PARTY_DIR}/libsodium/include/sodium.h")
        target_include_directories(hypercom_sodium SYSTEM INTERFACE
            "${HYPERCOM_THIRD_PARTY_DIR}/libsodium/include")
        find_library(HYPERCOM_SODIUM_LIBRARY
            NAMES sodium libsodium
            PATHS "${HYPERCOM_THIRD_PARTY_DIR}/libsodium/lib"
            NO_DEFAULT_PATH)
        if(HYPERCOM_SODIUM_LIBRARY)
            target_link_libraries(hypercom_sodium INTERFACE
                "${HYPERCOM_SODIUM_LIBRARY}")
            if(WIN32)
                target_compile_definitions(hypercom_sodium INTERFACE SODIUM_STATIC)
            endif()
            message(STATUS "hypercom: vendored libsodium -> third_party/libsodium")
            return()
        endif()
    endif()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SODIUM QUIET IMPORTED_TARGET libsodium)
        if(SODIUM_FOUND)
            target_link_libraries(hypercom_sodium INTERFACE PkgConfig::SODIUM)
            message(STATUS "hypercom: system libsodium -> ${SODIUM_VERSION}")
            return()
        endif()
    endif()
    hypercom_fail_missing_dependency("libsodium"
        "apt install libsodium-dev  /  brew install libsodium")
endfunction()

function(hypercom_require_sqlite3)
    if(TARGET hypercom::sqlite3)
        return()
    endif()
    # Preferred mode: amalgamation compiled into the binary.
    if(EXISTS "${HYPERCOM_THIRD_PARTY_DIR}/sqlite3/sqlite3.c")
        add_library(hypercom_sqlite3 STATIC
            "${HYPERCOM_THIRD_PARTY_DIR}/sqlite3/sqlite3.c")
        # SYSTEM: sqlite3.h itself uses an old-style cast internally. Marking
        # the include path as a system header suppresses our warning flags
        # for it specifically, in every file that includes it -- silencing
        # hypercom_sqlite3's own compile only covers sqlite3.c, not callers.
        target_include_directories(hypercom_sqlite3 SYSTEM PUBLIC
            "${HYPERCOM_THIRD_PARTY_DIR}/sqlite3")
        target_compile_definitions(hypercom_sqlite3 PUBLIC
            SQLITE_THREADSAFE=1
            SQLITE_DEFAULT_WAL_SYNCHRONOUS=1
            SQLITE_ENABLE_FTS5=0
            SQLITE_OMIT_LOAD_EXTENSION=1
            SQLITE_DQS=0
            SQLITE_MAX_EXPR_DEPTH=100
            SQLITE_DEFAULT_FOREIGN_KEYS=1)
        hypercom_silence_third_party(hypercom_sqlite3)
        add_library(hypercom::sqlite3 ALIAS hypercom_sqlite3)
        message(STATUS "hypercom: amalgamated sqlite3 -> third_party/sqlite3")
        return()
    endif()
    find_package(SQLite3 QUIET)
    if(SQLite3_FOUND)
        add_library(hypercom_sqlite3 INTERFACE)
        target_link_libraries(hypercom_sqlite3 INTERFACE SQLite::SQLite3)
        add_library(hypercom::sqlite3 ALIAS hypercom_sqlite3)
        message(STATUS "hypercom: system sqlite3 -> ${SQLite3_VERSION}")
        return()
    endif()
    hypercom_fail_missing_dependency("sqlite3"
        "apt install libsqlite3-dev")
endfunction()

# miniaudio: welcome theme playback, graphical client only.
#
# FOURTH DEPENDENCY, outside the three allowed by the project's norm.
# Justification: getting an MP3 out to a sound card requires either a
# library, or a homegrown decoder paired with two platform backends
# (WASAPI, ALSA). miniaudio fits in a single public-domain header, only
# touches hypercom_client, and its absence breaks nothing -- the client
# then builds with audio_player_silent.cpp and the intro plays without
# sound.
function(hypercom_try_miniaudio out_found)
    set(miniaudio_header "${HYPERCOM_THIRD_PARTY_DIR}/miniaudio/miniaudio.h")
    if(NOT EXISTS "${miniaudio_header}")
        message(STATUS
            "hypercom: audio disabled -- third_party/miniaudio missing. "
            "The interface works, the intro plays without music.")
        set(${out_found} FALSE PARENT_SCOPE)
        return()
    endif()
    if(NOT TARGET hypercom::miniaudio)
        add_library(hypercom_miniaudio STATIC
            "${CMAKE_SOURCE_DIR}/client/ui/miniaudio_implementation.c")
        target_include_directories(hypercom_miniaudio SYSTEM PUBLIC
            "${HYPERCOM_THIRD_PARTY_DIR}/miniaudio")
        hypercom_silence_third_party(hypercom_miniaudio)
        if(UNIX AND NOT APPLE)
            find_package(Threads REQUIRED)
            target_link_libraries(hypercom_miniaudio PUBLIC
                Threads::Threads ${CMAKE_DL_LIBS} m)
        endif()
        add_library(hypercom::miniaudio ALIAS hypercom_miniaudio)
    endif()
    set(${out_found} TRUE PARENT_SCOPE)
endfunction()

# ImGui is optional: its absence only disables the graphical client, it
# must never break building the server or the CLI client.
#
# TENSION WITH THE PROJECT'S NORM, explicitly flagged:
# Dear ImGui doesn't draw anything on its own, it requires a
# platform/rendering backend.
#
# Choice made: glfw3 + OpenGL 3, on Windows JUST AS on Linux.
#
# The alternative was a per-platform backend (Win32/DX11 on one side,
# GLFW/OpenGL on the other), which wouldn't have added any dependency on
# Windows. It was ruled out: two backends means two initialization
# paths, two event loops, and twice the code to audit, for an interface
# that displays text in panels.
#
# glfw3 is therefore the only dependency outside the three allowed by
# the project's norm. It concerns ONLY the graphical client: neither the
# server, the CLI client, nor the common library depend on it, and their
# build stays intact if it's absent.
#   Linux   : apt install libglfw3-dev
#   Windows : vcpkg install glfw3, or official binaries from glfw.org
function(hypercom_try_imgui out_found)
    set(imgui_root "${HYPERCOM_THIRD_PARTY_DIR}/imgui")
    if(NOT EXISTS "${imgui_root}/imgui.cpp")
        message(STATUS "hypercom: graphical client skipped -- third_party/imgui missing")
        set(${out_found} FALSE PARENT_SCOPE)
        return()
    endif()
    if(TARGET hypercom::imgui)
        set(${out_found} TRUE PARENT_SCOPE)
        return()
    endif()
    set(imgui_sources
        "${imgui_root}/imgui.cpp"
        "${imgui_root}/imgui_draw.cpp"
        "${imgui_root}/imgui_tables.cpp"
        "${imgui_root}/imgui_widgets.cpp")
    if(EXISTS "${HYPERCOM_THIRD_PARTY_DIR}/glfw/include/GLFW/glfw3.h")
        find_library(HYPERCOM_GLFW_LIBRARY
            NAMES glfw3 glfw3dll glfw
            PATHS "${HYPERCOM_THIRD_PARTY_DIR}/glfw/lib-vc2022"
                  "${HYPERCOM_THIRD_PARTY_DIR}/glfw/lib-vc2019"
                  "${HYPERCOM_THIRD_PARTY_DIR}/glfw/lib"
            NO_DEFAULT_PATH)
        if(HYPERCOM_GLFW_LIBRARY)
            if(NOT TARGET hypercom_glfw)
                add_library(hypercom_glfw INTERFACE)
                target_include_directories(hypercom_glfw SYSTEM INTERFACE "${HYPERCOM_THIRD_PARTY_DIR}/glfw/include")
                target_link_libraries(hypercom_glfw INTERFACE "${HYPERCOM_GLFW_LIBRARY}")
            endif()
            set(glfw3_FOUND TRUE)
        endif()
    endif()
    if(NOT glfw3_FOUND)
        find_package(glfw3 QUIET)
        if(NOT glfw3_FOUND)
            find_package(PkgConfig QUIET)
            if(PkgConfig_FOUND)
                pkg_check_modules(GLFW3 QUIET IMPORTED_TARGET glfw3)
            endif()
        endif()
    endif()
    if(NOT glfw3_FOUND AND NOT GLFW3_FOUND)
        message(STATUS
            "hypercom: graphical client skipped -- glfw3 missing. "
            "The server and the CLI client still get built.")
        set(${out_found} FALSE PARENT_SCOPE)
        return()
    endif()
    find_package(OpenGL REQUIRED)
    list(APPEND imgui_sources
        "${imgui_root}/backends/imgui_impl_glfw.cpp"
        "${imgui_root}/backends/imgui_impl_opengl3.cpp")
    add_library(hypercom_imgui STATIC ${imgui_sources})
    target_include_directories(hypercom_imgui SYSTEM PUBLIC
        "${imgui_root}" "${imgui_root}/backends")
    if(TARGET hypercom_glfw)
        target_link_libraries(hypercom_imgui PUBLIC hypercom_glfw OpenGL::GL)
    elseif(glfw3_FOUND)
        target_link_libraries(hypercom_imgui PUBLIC glfw OpenGL::GL)
    else()
        target_link_libraries(hypercom_imgui PUBLIC PkgConfig::GLFW3 OpenGL::GL)
    endif()
    hypercom_silence_third_party(hypercom_imgui)
    add_library(hypercom::imgui ALIAS hypercom_imgui)
    set(${out_found} TRUE PARENT_SCOPE)
endfunction()
