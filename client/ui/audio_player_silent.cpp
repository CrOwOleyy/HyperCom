#include "client/ui/audio_player.hpp"

namespace hypercom::client {

// Fallback compiled in place of audio_player.cpp when miniaudio isn't
// vendored.
//
// It exists so gui_main.cpp needs no #ifdef at all: the intro plays out
// the same way, driven by the clock, just without music. Conditional
// compilation scattered through the drawing code would be far more
// painful to read than a second ten-line file.
struct audio_player::engine_state {};

audio_player::audio_player() : state_{std::make_unique<engine_state>()}
{}

audio_player::~audio_player() = default;

bool audio_player::start_track(std::string const &, std::string &error_out)
{
    error_out = "client construit sans support audio";
    return false;
}

void audio_player::stop_track()
{}

double audio_player::get_track_length_seconds() const
{
    return 0.0;
}

} // namespace hypercom::client
