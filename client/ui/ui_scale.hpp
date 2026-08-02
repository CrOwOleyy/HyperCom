#pragma once

struct GLFWwindow;

namespace hypercom::client {

// Mise a l'echelle de l'interface.
//
// Le probleme que ce module resout : la police integree a Dear ImGui est un
// bitmap de 13 pixels. Sur un ecran dense ou simplement grand, elle est
// illisible, et l'agrandir avec FontGlobalScale ne fait que l'etaler -- on
// obtient du gros flou, pas du gros net.
//
// La solution est de RASTERISER la police a la bonne taille plutot que de
// zoomer une image. L'atlas est donc reconstruit a chaque changement
// d'echelle, et uniquement a ce moment-la : c'est une operation couteuse qui
// n'a rien a faire dans une boucle de rendu.
//
// L'echelle finale combine deux facteurs :
//   display_scale  deduit du systeme (DPI, puis resolution en secours)
//   user_zoom      regle par l'utilisateur, parce qu'aucune heuristique ne
//                  connait sa distance a l'ecran ni sa vue

constexpr float MIN_USER_ZOOM = 0.7f;
constexpr float MAX_USER_ZOOM = 2.5f;
constexpr float USER_ZOOM_STEP = 0.1f;

// 17 px plutot que les 13 px d'origine : meme sans aucune mise a l'echelle,
// la valeur par defaut doit deja etre confortable.
constexpr float BASE_FONT_SIZE = 17.0f;

struct ui_scale_state {
    float display_scale = 1.0f;
    float user_zoom = 1.0f;
    float applied_scale = 0.0f;
    bool font_rebuild_needed = true;
};

void detect_display_scale(GLFWwindow *window, ui_scale_state &state);

[[nodiscard]] float compute_effective_scale(ui_scale_state const &state);

// Ctrl + / Ctrl - / Ctrl 0, et Ctrl + molette. A appeler pendant une image :
// le drapeau de reconstruction est traite au tour suivant, entre deux images.
[[nodiscard]] bool handle_zoom_input(ui_scale_state &state);

// A n'appeler QU'ENTRE deux images, jamais entre NewFrame et Render.
void rebuild_scaled_font(ui_scale_state &state);

void draw_zoom_controls(ui_scale_state &state);

} // namespace hypercom::client
