# Applique la discipline de compilation exigee par BRIEF.md 10.
#
#   -Wall -Wextra -Werror -fstack-protector-strong -D_FORTIFY_SOURCE=2
#   -fPIE -pie -Wl,-z,relro,-z,now
#
# L'equivalent MSVC est fourni pour le client Windows : /W4 /WX /GS /guard:cf,
# plus les protections de l'editeur de liens (ASLR, DEP, entropie haute).

# HYPERCOM_PEDANTIC ajoute un second cercle d'avertissements (-Wconversion,
# -Wold-style-cast, -Wshadow...). Il est separe du jeu impose par le brief pour
# une raison pratique : combine a -Werror, il transforme le moindre durcissement
# de compilateur en build casse. Il est fait pour tourner en integration, pas
# pour bloquer un developpeur qui change de version de GCC.
option(HYPERCOM_PEDANTIC "Avertissements etendus, au-dela du jeu impose" OFF)

function(hypercom_apply_strict_options target)
    if(MSVC)
        target_compile_options(${target} PRIVATE
            /W4 /WX /permissive- /GS /guard:cf /Zc:__cplusplus /utf-8
            /D_CRT_SECURE_NO_WARNINGS)
        target_link_options(${target} PRIVATE
            /DYNAMICBASE /NXCOMPAT /HIGHENTROPYVA /guard:cf)
    else()
        # Jeu impose par BRIEF.md 10, non negociable.
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Werror -fstack-protector-strong)
        if(HYPERCOM_PEDANTIC)
            target_compile_options(${target} PRIVATE
                -Wshadow -Wconversion -Wsign-conversion -Wcast-qual
                -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual
                -Wdouble-promotion -Wformat=2 -Wundef)
        endif()
        # _FORTIFY_SOURCE exige une optimisation active : l'activer en Debug
        # ne ferait que produire un avertissement du preprocesseur.
        target_compile_definitions(${target} PRIVATE
            $<$<NOT:$<CONFIG:Debug>>:_FORTIFY_SOURCE=2>)
        if(NOT APPLE)
            target_link_options(${target} PRIVATE
                -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack)
        endif()
    endif()
endfunction()

# Les executables sont en plus lies en PIE (ASLR complet).
function(hypercom_apply_executable_hardening target)
    hypercom_apply_strict_options(${target})
    if(NOT MSVC)
        set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ON)
        target_link_options(${target} PRIVATE -pie)
    endif()
endfunction()

# Le code tiers (sqlite3, imgui) ne passe pas -Werror : il n'est pas sous notre
# controle et sa norme n'est pas la notre.
function(hypercom_silence_third_party target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W0)
    else()
        target_compile_options(${target} PRIVATE -w)
    endif()
endfunction()
