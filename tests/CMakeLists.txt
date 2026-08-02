# Tests unitaires ecrits a la main : BRIEF.md 15 n'autorise pas de framework
# de test supplementaire, et le besoin ne le justifie pas.

add_library(hypercom_test_harness STATIC test_harness.cpp)
target_include_directories(hypercom_test_harness PUBLIC "${CMAKE_SOURCE_DIR}")
hypercom_apply_strict_options(hypercom_test_harness)

function(hypercom_add_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE
        hypercom_test_harness hypercom_common)
    hypercom_apply_strict_options(${name})
    hypercom_apply_sanitizers(${name})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

hypercom_add_test(protocol_parsing_test)
hypercom_add_test(crypto_round_trip_test)
