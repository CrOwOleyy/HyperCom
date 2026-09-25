# Applies the compilation discipline required by the project's norm.
#
#   -Wall -Wextra -Werror -fstack-protector-strong -D_FORTIFY_SOURCE=2
#   -fPIE -pie -Wl,-z,relro,-z,now
#
# The MSVC equivalent is provided for the Windows client: /W4 /WX /GS
# /guard:cf, plus the linker protections (ASLR, DEP, high entropy).

# HYPERCOM_PEDANTIC adds a second circle of warnings (-Wconversion,
# -Wold-style-cast, -Wshadow...). It's kept separate from the mandatory
# set for a practical reason: combined with -Werror, it turns the
# slightest compiler upgrade into a broken build. It's meant to run in
# integration, not to block a developer switching GCC versions.
option(HYPERCOM_PEDANTIC "Extended warnings, beyond the mandatory set" OFF)

function(hypercom_apply_strict_options target)
    if(MSVC)
        target_compile_options(${target} PRIVATE
            /W4 /WX /permissive- /GS /guard:cf /Zc:__cplusplus /utf-8
            /D_CRT_SECURE_NO_WARNINGS)
        target_link_options(${target} PRIVATE
            /DYNAMICBASE /NXCOMPAT /HIGHENTROPYVA /guard:cf)
    else()
        # Set required by the project's norm, non-negotiable.
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Werror -fstack-protector-strong)
        if(HYPERCOM_PEDANTIC)
            target_compile_options(${target} PRIVATE
                -Wshadow -Wconversion -Wsign-conversion -Wcast-qual
                -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual
                -Wdouble-promotion -Wformat=2 -Wundef)
        endif()
        # _FORTIFY_SOURCE requires optimization to be active: enabling it
        # in Debug would only produce a preprocessor warning.
        target_compile_definitions(${target} PRIVATE
            $<$<NOT:$<CONFIG:Debug>>:_FORTIFY_SOURCE=2>)
        if(NOT APPLE)
            target_link_options(${target} PRIVATE
                -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack)
        endif()
    endif()
endfunction()

# Executables are additionally linked as PIE (full ASLR).
function(hypercom_apply_executable_hardening target)
    hypercom_apply_strict_options(${target})
    if(NOT MSVC)
        set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ON)
        target_link_options(${target} PRIVATE -pie)
    endif()
endfunction()

# Third-party code (sqlite3, imgui) doesn't get -Werror: it's not under
# our control and its norm isn't ours.
function(hypercom_silence_third_party target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W0)
    else()
        target_compile_options(${target} PRIVATE -w)
    endif()
endfunction()
