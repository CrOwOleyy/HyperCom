# Dedicated ASAN / UBSAN / TSAN build targets required by the project's
# norm.
#
# Usage:
#   cmake -B build-asan -DHYPERCOM_SANITIZER=address,undefined
#   cmake -B build-tsan -DHYPERCOM_SANITIZER=thread
#
# ASAN and TSAN are mutually exclusive: configuration fails rather than
# producing a silently unusable binary.

set(HYPERCOM_SANITIZER "" CACHE STRING
    "Active sanitizers: address, undefined, thread, or a comma-separated list")

function(hypercom_validate_sanitizer_choice choice)
    string(FIND "${choice}" "thread" thread_position)
    string(FIND "${choice}" "address" address_position)
    if(NOT thread_position EQUAL -1 AND NOT address_position EQUAL -1)
        message(FATAL_ERROR
            "hypercom: -fsanitize=thread and -fsanitize=address are "
            "incompatible. Use two separate build directories.")
    endif()
endfunction()

function(hypercom_apply_sanitizers target)
    if(HYPERCOM_SANITIZER STREQUAL "")
        return()
    endif()
    if(MSVC)
        message(WARNING
            "hypercom: only address is available under MSVC, "
            "HYPERCOM_SANITIZER=${HYPERCOM_SANITIZER} partially ignored.")
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
    message(STATUS "hypercom: active sanitizers -> ${HYPERCOM_SANITIZER}")
endif()
