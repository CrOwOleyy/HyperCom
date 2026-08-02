#pragma once

#include <cstdint>

namespace hypercom::client {

// La sequence d'accueil, jouee UNE seule fois, a la creation du compte.
//
//   0 s ........ 6 s          message de bienvenue, sur la musique
//   6 s ........ fin du son   l'interface apparait bloc par bloc, en bulles
//   ensuite                   interface normale, plus aucune animation
//
// Le decoupage suit le morceau : six secondes d'intro, puis la partie rythmee.
// Si la duree reelle du fichier n'est pas lisible, on retombe sur une valeur
// fixe -- l'animation ne doit jamais dependre de la reussite de l'audio.
enum class intro_phase : std::uint8_t {
    welcome,
    reveal,
    finished,
};

constexpr double INTRO_WELCOME_SECONDS = 6.0;
constexpr double INTRO_FALLBACK_TOTAL_SECONDS = 12.5;

struct intro_state {
    bool active = false;
    double elapsed_seconds = 0.0;
    double total_seconds = INTRO_FALLBACK_TOTAL_SECONDS;
};

void begin_intro(intro_state &state, double track_seconds);

void advance_intro(intro_state &state, double delta_seconds);

[[nodiscard]] intro_phase get_intro_phase(intro_state const &state);

// Progression 0..1 du bloc `index` parmi `count`. Rend 1 hors sequence, pour
// que les fonctions de dessin n'aient pas a savoir si une intro est en cours.
[[nodiscard]] float compute_element_reveal(intro_state const &state, int index,
                                           int count);

} // namespace hypercom::client
