# Cibles de build dediees ASAN / UBSAN / TSAN exigees par BRIEF.md 10.
#
# Utilisation :
#   cmake -B build-asan -DHYPERCOM_SANITIZER=address,undefined
#   cmake -B build-tsan -DHYPERCOM_SANITIZER=thread
#
# ASAN et TSAN sont mutuellement exclusifs : la configuration echoue plutot que
# de produire un binaire silencieusement inutilisable.

set(HYPERCOM_SANITIZER "" CACHE STRING
    "Sanitizers actifs : address, undefined, thread, ou une liste separee par des virgules")

function(hypercom_validate_sanitizer_choice choice)
    string(FIND "${choice}" "thread" thread_position)
    string(FIND "${choice}" "address" address_position)
    if(NOT thread_position EQUAL -1 AND NOT address_position EQUAL -1)
        message(FATAL_ERROR
            "hypercom: -fsanitize=thread et -fsanitize=address sont "
            "incompatibles. Utiliser deux repertoires de build distincts.")
    endif()
endfunction()

function(hypercom_apply_sanitizers target)
    if(HYPERCOM_SANITIZER STREQUAL "")
        return()
    endif()
    if(MSVC)
        message(WARNING
            "hypercom: seul address est disponible sous MSVC, "
            "HYPERCOM_SANITIZER=${HYPERCOM_SANITIZER} partiellement ignore.")
        target_compile_options(${target} PRIVATE /fsanitize=address)
        return()
    endif()
    hypercom_validate_sanitizer_choice("${HYPERCOM_SANITIZER}")
    target_compile_options(${target} PRIVATE
        -fsanitize=${HYPERCOM_SANITIZER}
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls)
    target_link_options(${target} PRIVATE -fsanitize=${HYPERCOM_SANITIZER})
endfunction()

if(NOT HYPERCOM_SANITIZER STREQUAL "")
    message(STATUS "hypercom: sanitizers actifs -> ${HYPERCOM_SANITIZER}")
endif()
