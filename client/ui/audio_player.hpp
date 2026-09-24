#pragma once

#include <memory>
#include <string>

namespace hypercom::client {

// Playback of the welcome theme.
//
// miniaudio is the project's only dependency besides libsodium, SQLite
// and ImGui, and it's confined to the graphical client: neither the
// server, the CLI client, nor the common library ever see it.
//
// No failure is fatal. Machine without a sound card, missing audio
// server, device already in use, file not found: in all these cases
// the client still opens normally, silently. The music is a nicety,
// not a functional requirement.
class audio_player {
public:
    audio_player();

    ~audio_player();

    // file_name is looked up next to the executable and then in the
    // parent directories, which covers both an installed binary and a
    // local build.
    [[nodiscard]] bool start_track(std::string const &file_name,
                                   std::string &error_out);

    void stop_track();

    // 0 if the track couldn't be opened. The caller then falls back to
    // a fixed duration instead of playing a zero-length animation.
    [[nodiscard]] double get_track_length_seconds() const;

private:
    struct engine_state;

    std::unique_ptr<engine_state> state_;
};

} // namespace hypercom::client
