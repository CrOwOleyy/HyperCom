add_executable(hypercom_keygen keygen/keygen_main.cpp)
target_link_libraries(hypercom_keygen PRIVATE hypercom_common)
hypercom_apply_executable_hardening(hypercom_keygen)
hypercom_apply_sanitizers(hypercom_keygen)

# Harnais de fuzzing : libFuzzer, donc clang uniquement.
#
# Il ne lie QUE hypercom_protocol, sans libsodium ni sqlite. Ce n'est pas une
# economie, c'est la raison d'etre du decoupage : le parseur se fuzze en
# isolation totale, ce qui rend les campagnes rapides et les plantages
# univoques.
if(HYPERCOM_BUILD_FUZZ)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR
            "hypercom: HYPERCOM_BUILD_FUZZ exige clang++ "
            "(-DCMAKE_CXX_COMPILER=clang++)")
    endif()
    add_executable(fuzz_protocol_frame fuzz/fuzz_protocol_frame.cpp)
    target_link_libraries(fuzz_protocol_frame PRIVATE hypercom_protocol)
    target_compile_options(fuzz_protocol_frame PRIVATE
        -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer)
    target_link_options(fuzz_protocol_frame PRIVATE
        -fsanitize=fuzzer,address,undefined)
endif()
