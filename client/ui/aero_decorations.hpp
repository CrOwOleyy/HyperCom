#pragma once

#include <imgui.h>

namespace hypercom::client {

// Le decor Frutiger Aero : degrade, bulles, reflets.
//
// Tout est peint directement dans une ImDrawList plutot que par le style ImGui,
// parce qu'ImGui ne sait pas faire de degrade ni de cercle decoratif.
//
// Les bulles n'ont AUCUN etat : leur position est une fonction pure de leur
// indice et du temps. Pas de tableau a faire vivre entre deux images, donc pas
// de globale mutable non plus (regle G4).

constexpr int AERO_BUBBLE_COUNT = 22;

void draw_aero_backdrop(ImDrawList *list, ImVec2 origin, ImVec2 size,
                        float time_seconds);

// Pastille de verre : remplissage translucide, liseré clair, reflet en haut.
void draw_glass_surface(ImDrawList *list, ImVec2 minimum, ImVec2 maximum,
                        float rounding);

// Le reflet seul, a poser sur un widget deja dessine par ImGui.
void draw_gloss_highlight(ImDrawList *list, ImVec2 minimum, ImVec2 maximum,
                          float rounding);

} // namespace hypercom::client
