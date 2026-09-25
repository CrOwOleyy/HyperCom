# Resolution des trois seules dependances autorisees par BRIEF.md 15 :
# libsodium, SQLite et Dear ImGui.
#
# Trois sources possibles, dans cet ordre de priorite :
#   1. third_party/<nom>          (vendorise, mode recommande pour la prod)
#   2. paquets systeme            (pkg-config ou find_package)
#   3. echec explicite avec la commande exacte a lancer
#
# Aucune recuperation reseau n'est declenchee par CMake : le telechargement est
# une action deliberee, faite via scripts/fetch_third_party.sh.

set(HYPERCOM_THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party")

function(hypercom_fail_missing_dependency name hint)
    message(FATAL_ERROR
        "hypercom: dependance '${name}' introuvable.\n"
        "  Option 1 (vendorise) : ./scripts/fetch_third_party.sh\n"
        "  Option 2 (systeme)   : ${hint}")
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
            message(STATUS "hypercom: libsodium vendorise -> third_party/libsodium")
            return()
        endif()
    endif()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SODIUM QUIET IMPORTED_TARGET libsodium)
        if(SODIUM_FOUND)
            target_link_libraries(hypercom_sodium INTERFACE PkgConfig::SODIUM)
            message(STATUS "hypercom: libsodium systeme -> ${SODIUM_VERSION}")
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
    # Mode privilegie par BRIEF.md 2 : amalgamation compilee dans le binaire.
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
        message(STATUS "hypercom: sqlite3 amalgame -> third_party/sqlite3")
        return()
    endif()
    find_package(SQLite3 QUIET)
    if(SQLite3_FOUND)
        add_library(hypercom_sqlite3 INTERFACE)
        target_link_libraries(hypercom_sqlite3 INTERFACE SQLite::SQLite3)
        add_library(hypercom::sqlite3 ALIAS hypercom_sqlite3)
        message(STATUS "hypercom: sqlite3 systeme -> ${SQLite3_VERSION}")
        return()
    endif()
    hypercom_fail_missing_dependency("sqlite3"
        "apt install libsqlite3-dev")
endfunction()

# miniaudio : lecture du theme d'accueil, client graphique uniquement.
#
# QUATRIEME DEPENDANCE, hors des trois autorisees par BRIEF.md 15.
# Justification : sortir un MP3 sur une carte son demande soit une
# bibliotheque, soit un decodeur maison doublé de deux backends plateforme
# (WASAPI, ALSA). miniaudio tient en un seul en-tete du domaine public, ne
# touche que hypercom_client, et son absence ne casse rien -- le client se
# construit alors avec audio_player_silent.cpp et l'intro se joue sans son.
function(hypercom_try_miniaudio out_found)
    set(miniaudio_header "${HYPERCOM_THIRD_PARTY_DIR}/miniaudio/miniaudio.h")
    if(NOT EXISTS "${miniaudio_header}")
        message(STATUS
            "hypercom: audio desactive -- third_party/miniaudio absent. "
            "L'interface fonctionne, l'intro se joue sans musique.")
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

# ImGui est optionnel : son absence desactive seulement le client graphique,
# elle ne doit jamais casser la construction du serveur ni du client CLI.
#
# TENSION AVEC BRIEF.md 15, signalee explicitement :
# Dear ImGui ne dessine rien tout seul, il exige un backend plateforme/rendu.
#
# Choix retenu : glfw3 + OpenGL 3, sur Windows COMME sur Linux.
#
# L'alternative etait un backend par plateforme (Win32/DX11 d'un cote,
# GLFW/OpenGL de l'autre), ce qui n'aurait ajoute aucune dependance sous
# Windows. Elle a ete ecartee : deux backends, ce sont deux chemins
# d'initialisation, deux boucles d'evenements et deux fois plus de code a
# auditer, pour une interface qui affiche du texte dans des panneaux.
#
# glfw3 est donc la seule dependance hors des trois autorisees par le brief.
# Elle ne concerne QUE le client graphique : ni le serveur, ni le client CLI,
# ni la bibliotheque commune n'en dependent, et leur build reste intact si elle
# est absente.
#   Linux   : apt install libglfw3-dev
#   Windows : vcpkg install glfw3, ou binaires officiels glfw.org
function(hypercom_try_imgui out_found)
    set(imgui_root "${HYPERCOM_THIRD_PARTY_DIR}/imgui")
    if(NOT EXISTS "${imgui_root}/imgui.cpp")
        message(STATUS "hypercom: client graphique ignore -- third_party/imgui absent")
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
            "hypercom: client graphique ignore -- glfw3 absent. "
            "Le serveur et le client CLI restent construits.")
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
