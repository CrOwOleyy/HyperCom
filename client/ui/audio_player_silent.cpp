#include "client/ui/audio_player.hpp"

namespace hypercom::client {

// Repli compile a la place de audio_player.cpp quand miniaudio n'est pas
// vendorise.
//
// Il existe pour que gui_main.cpp n'ait aucun #ifdef : l'intro se deroule de la
// meme facon, pilotee par l'horloge, simplement sans musique. Une compilation
// conditionnelle disseminee dans le code de dessin serait bien plus penible a
// relire qu'un second fichier de dix lignes.
struct audio_player::engine_state {};

audio_player::audio_player() : state_{std::make_unique<engine_state>()} {}

audio_player::~audio_player() = default;

bool audio_player::start_track(std::string const &, std::string &error_out)
{
    error_out = "client construit sans support audio";
    return false;
}

void audio_player::stop_track() {}

double audio_player::get_track_length_seconds() const
{
    return 0.0;
}

} // namespace hypercom::client
