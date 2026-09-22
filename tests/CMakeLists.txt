# Tests unitaires ecrits a la main : le projet n'autorise pas de framework de
# test supplementaire, et le besoin ne le justifie pas.

add_library(hypercom_test_harness STATIC test_harness.cpp)
target_include_directories(hypercom_test_harness PUBLIC "${CMAKE_SOURCE_DIR}")
hypercom_apply_strict_options(hypercom_test_harness)

# extra_sources : fichiers supplementaires du meme executable, pour un test
# qui a besoin d'un module de support (ex. noise_vector_replay.cpp) sans faire
# gonfler le fichier de test lui-meme au-dela de la regle O3.
function(hypercom_add_test name)
    add_executable(${name} ${name}.cpp ${ARGN})
    target_link_libraries(${name} PRIVATE
        hypercom_test_harness hypercom_common)
    hypercom_apply_strict_options(${name})
    hypercom_apply_sanitizers(${name})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

hypercom_add_test(protocol_parsing_test)
hypercom_add_test(crypto_round_trip_test)
hypercom_add_test(noise_official_vectors_test noise_vector_replay.cpp)
hypercom_add_test(message_roundtrip_session_test)
hypercom_add_test(message_roundtrip_content_test)
hypercom_add_test(message_roundtrip_social_test)

# Multi-serveurs : scellement generique, derivation d'une identite par serveur,
# lien d'invitation et registre chiffre. Lie le coeur client, qui porte le
# keystore.
hypercom_add_test(multi_server_identity_test)
target_link_libraries(multi_server_identity_test PRIVATE hypercom_client_core)

# Rejeu du corpus de fuzzing : reutilise le point d'entree libFuzzer sans
# libFuzzer, pour que les cas limites connus tournent aussi sous gcc.
hypercom_add_test(fuzz_corpus_replay_test
    "${CMAKE_SOURCE_DIR}/tools/fuzz/fuzz_protocol_frame.cpp")
target_compile_definitions(fuzz_corpus_replay_test PRIVATE
    "HYPERCOM_FUZZ_CORPUS_DIR=\"${CMAKE_SOURCE_DIR}/tools/fuzz/corpus\"")

# Reconnexion : exerce le vrai server_connection contre un repondeur Noise sur
# la boucle locale. Le repondeur de test utilise les sockets POSIX -- le
# serveur du projet se developpe sous WSL2, doubler ce module pour Winsock
# n'apporterait rien.
if(NOT WIN32)
    hypercom_add_test(reconnection_test loopback_noise_server.cpp)
    find_package(Threads REQUIRED)
    target_link_libraries(reconnection_test PRIVATE
        hypercom_client_core Threads::Threads)
endif()
