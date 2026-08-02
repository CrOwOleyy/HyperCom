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
