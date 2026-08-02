#pragma once

#include <imgui.h>

namespace hypercom::client {

// Palette verte pale. L'idee est un outil de labo, pas un produit : pas de
// degrade, pas d'ombre portee, des angles a peine adoucis.
//
// Attention en ajoutant des couleurs : il y a deux familles.
// Les vives (ACCENT, ACCENT_HOVER) servent aux APLATS -- boutons, selection.
// Les sombres (_TEXT) servent au TEXTE. Ne pas les intervertir : pose sur le
// fond clair, l'accent vif donne un contraste de 2.96:1 la ou le WCAG
// AA en demande 4.5. Autrement dit, illisible pour beaucoup de monde.
// Les valeurs ci-dessous sont mesurees, pas estimees.

// Fonds.
constexpr ImVec4 PALE_GREEN_CANVAS{0.882f, 0.921f, 0.863f, 1.0f};
constexpr ImVec4 PALE_GREEN_PANEL{0.949f, 0.968f, 0.937f, 1.0f};
constexpr ImVec4 PALE_GREEN_SUNKEN{0.831f, 0.882f, 0.808f, 1.0f};
constexpr ImVec4 PALE_GREEN_BORDER{0.725f, 0.796f, 0.682f, 1.0f};

// Accent vif -- aplats uniquement.
constexpr ImVec4 PALE_GREEN_ACCENT{0.431f, 0.607f, 0.384f, 1.0f};
constexpr ImVec4 PALE_GREEN_ACCENT_HOVER{0.521f, 0.702f, 0.466f, 1.0f};
constexpr ImVec4 PALE_GREEN_ACCENT_ACTIVE{0.352f, 0.517f, 0.309f, 1.0f};

// Textes. Les trois passent le AA sur les deux fonds (6.1:1, 5.2:1, 12.7:1).
constexpr ImVec4 PALE_GREEN_ACCENT_TEXT{0.243f, 0.400f, 0.204f, 1.0f};
constexpr ImVec4 PALE_GREEN_INK{0.149f, 0.188f, 0.121f, 1.0f};
constexpr ImVec4 PALE_GREEN_INK_MUTED{0.360f, 0.419f, 0.325f, 1.0f};

// Alerte : cle non epinglee, message illisible, identite non verifiee.
// Rouge sombre plutot qu'orange -- l'orange etait a 4.47:1, juste sous la
// barre, et surtout trop proche du vert d'accent pour un daltonien.
constexpr ImVec4 PALE_GREEN_ALERT{0.545f, 0.145f, 0.145f, 1.0f};
constexpr ImVec4 PALE_GREEN_WARNING{0.510f, 0.365f, 0.055f, 1.0f};

void apply_pale_green_theme(ImGuiStyle &style, float scale = 1.0f);

// Titre de section : capitale sourde suivie d'un filet.
void draw_section_heading(char const *label);

// Etat de la session.
//
// Le marqueur est [ok] ou [!], en toutes lettres, et le libelle est toujours
// affiche. La couleur ne fait que confirmer -- un vert et un rouge sont
// indistinguables pour environ 8 % des hommes, donc rien d'important ne doit
// dependre de la teinte seule. Meme regle partout ailleurs dans l'UI.
void draw_status_dot(bool secure, char const *label);

} // namespace hypercom::client
