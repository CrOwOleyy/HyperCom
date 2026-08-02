#pragma once

#include <imgui.h>

namespace hypercom::client {

// Direction artistique : Frutiger Aero.
//
// Verre translucide, degrade ciel vers aqua, bulles qui montent, reflets
// brillants. Le vert reste la couleur d'identite du projet, mais il vire au
// turquoise et gagne de la lumiere.
//
// Deux familles de couleurs, a ne pas melanger :
//   AERO_ACCENT*        aplats et remplissages
//   AERO_*_TEXT / INK   texte uniquement
//
// Les valeurs de texte sont mesurees, pas choisies a l'oeil. Le verre
// translucide est un piege classique : le fond varie sous le panneau, donc le
// contraste a ete verifie contre les trois pires cas (verre, haut et bas du
// degrade). Le minimum obtenu est 5.21:1, pour un seuil WCAG AA de 4.5:1.

// Fond : degrade dessine a la main, pas une couleur unie.
constexpr ImVec4 AERO_SKY_TOP{0.749f, 0.914f, 1.000f, 1.0f};
constexpr ImVec4 AERO_SKY_BOTTOM{0.788f, 0.949f, 0.847f, 1.0f};

// Verre.
constexpr ImVec4 AERO_GLASS{1.000f, 1.000f, 1.000f, 0.55f};
constexpr ImVec4 AERO_GLASS_STRONG{1.000f, 1.000f, 1.000f, 0.78f};
constexpr ImVec4 AERO_GLASS_SUNKEN{0.851f, 0.925f, 0.949f, 0.72f};
constexpr ImVec4 AERO_GLASS_BORDER{1.000f, 1.000f, 1.000f, 0.85f};

// Accent turquoise : aplats.
constexpr ImVec4 AERO_ACCENT{0.122f, 0.663f, 0.627f, 1.0f};
constexpr ImVec4 AERO_ACCENT_HOVER{0.310f, 0.808f, 0.769f, 1.0f};
constexpr ImVec4 AERO_ACCENT_ACTIVE{0.055f, 0.482f, 0.459f, 1.0f};

// Textes.
constexpr ImVec4 AERO_INK{0.063f, 0.188f, 0.220f, 1.0f};
constexpr ImVec4 AERO_INK_MUTED{0.235f, 0.376f, 0.408f, 1.0f};
constexpr ImVec4 AERO_ACCENT_TEXT{0.055f, 0.404f, 0.384f, 1.0f};
constexpr ImVec4 AERO_ALERT{0.612f, 0.129f, 0.129f, 1.0f};
constexpr ImVec4 AERO_WARNING{0.545f, 0.365f, 0.055f, 1.0f};

// Reflets et bulles.
constexpr ImVec4 AERO_GLOSS{1.000f, 1.000f, 1.000f, 0.45f};
constexpr ImVec4 AERO_BUBBLE{1.000f, 1.000f, 1.000f, 0.20f};
constexpr ImVec4 AERO_BUBBLE_RIM{1.000f, 1.000f, 1.000f, 0.42f};

void apply_aero_theme(ImGuiStyle &style, float scale = 1.0f);

// Titre de section : petite capitale turquoise suivie d'un filet.
void draw_section_heading(char const *label);

// Etat de la session.
//
// Le marqueur est [ok] ou [!], en toutes lettres, et le libelle est toujours
// affiche. La couleur ne fait que confirmer : un turquoise et un rouge sont
// indistinguables pour environ 8 % des hommes.
void draw_status_dot(bool secure, char const *label);

} // namespace hypercom::client
