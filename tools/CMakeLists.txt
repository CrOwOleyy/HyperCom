add_executable(hypercom_keygen keygen/keygen_main.cpp)
target_link_libraries(hypercom_keygen PRIVATE hypercom_common)
hypercom_apply_executable_hardening(hypercom_keygen)
hypercom_apply_sanitizers(hypercom_keygen)

# Admin CLI client. It links neither the crypto nor sqlite: it's just a
# pipe to the Unix socket, all the logic lives server-side.
add_executable(hypercom_adminctl adminctl/adminctl_main.cpp)
target_include_directories(hypercom_adminctl PRIVATE "${CMAKE_SOURCE_DIR}")
if(WIN32)
    target_link_libraries(hypercom_adminctl PRIVATE ws2_32)
endif()
hypercom_apply_executable_hardening(hypercom_adminctl)
hypercom_apply_sanitizers(hypercom_adminctl)

# Fuzzing harness: libFuzzer, so clang only.
#
# It links ONLY hypercom_protocol, no libsodium, no sqlite. That's not a
# shortcut, it's the whole point of the split: the parser gets fuzzed in
# total isolation, which keeps campaigns fast and crashes unambiguous.
if(HYPERCOM_BUILD_FUZZ)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR
            "hypercom: HYPERCOM_BUILD_FUZZ requires clang++ "
            "(-DCMAKE_CXX_COMPILER=clang++)")
    endif()
    add_executable(fuzz_protocol_frame fuzz/fuzz_protocol_frame.cpp)
    target_link_libraries(fuzz_protocol_frame PRIVATE hypercom_protocol)
    target_compile_options(fuzz_protocol_frame PRIVATE
        -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer)
    target_link_options(fuzz_protocol_frame PRIVATE
        -fsanitize=fuzzer,address,undefined)
endif()
