# Hand-written unit tests: the project doesn't allow an extra test
# framework, and the need doesn't justify one.

add_library(hypercom_test_harness STATIC test_harness.cpp)
target_include_directories(hypercom_test_harness PUBLIC "${CMAKE_SOURCE_DIR}")
hypercom_apply_strict_options(hypercom_test_harness)

# extra_sources: extra files for the same executable, for a test that
# needs a support module (e.g. noise_vector_replay.cpp) without bloating
# the test file itself past rule O3.
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

# Multi-server: generic sealing, per-server identity derivation, invite
# link and encrypted registry. Links the client core, which carries the
# keystore.
hypercom_add_test(multi_server_identity_test)
target_link_libraries(multi_server_identity_test PRIVATE hypercom_client_core)

# Fuzzing corpus replay: reuses the libFuzzer entry point without
# libFuzzer, so known edge cases also run under gcc.
hypercom_add_test(fuzz_corpus_replay_test
    "${CMAKE_SOURCE_DIR}/tools/fuzz/fuzz_protocol_frame.cpp")
target_compile_definitions(fuzz_corpus_replay_test PRIVATE
    "HYPERCOM_FUZZ_CORPUS_DIR=\"${CMAKE_SOURCE_DIR}/tools/fuzz/corpus\"")

# Reconnection: exercises the real server_connection against a Noise
# responder on the local loop. The test responder uses POSIX sockets --
# the project's server is developed under WSL2, duplicating this module
# for Winsock wouldn't add anything.
if(NOT WIN32)
    hypercom_add_test(reconnection_test loopback_noise_server.cpp)
    find_package(Threads REQUIRED)
    target_link_libraries(reconnection_test PRIVATE
        hypercom_client_core Threads::Threads)
endif()
