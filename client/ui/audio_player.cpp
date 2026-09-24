#include "client/ui/audio_player.hpp"

#include <filesystem>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include <miniaudio.h>
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

namespace hypercom::client {
namespace {

// The executable's directory, not the current working directory.
//
// The distinction matters: a double-click on Windows and a launch from
// a terminal don't give the same working directory, and looking for
// the MP3 relative to it makes it unfindable half the time.
[[nodiscard]] std::filesystem::path find_executable_directory()
{
    std::error_code failure;
#if defined(_WIN32)
    std::wstring buffer(4096, L'\0');
    DWORD const length = GetModuleFileNameW(nullptr, buffer.data(),
                                            static_cast<DWORD>(buffer.size()));
    if (length == 0 || length >= buffer.size()) {
        return std::filesystem::current_path(failure);
    }
    buffer.resize(length);
    return std::filesystem::path{buffer}.parent_path();
#else
    std::filesystem::path const target =
        std::filesystem::read_symlink("/proc/self/exe", failure);
    if (failure) {
        return std::filesystem::current_path(failure);
    }
    return target.parent_path();
#endif
}

[[nodiscard]] std::string find_asset_path(std::string const &file_name)
{
    std::filesystem::path const base = find_executable_directory();
    // Next to the binary first, then the parent directories to cover a
    // local build, then the current directory as a last resort.
    std::vector<std::filesystem::path> const candidates{
        base / file_name,
        base / "assets" / file_name,
        base / ".." / file_name,
        base / ".." / ".." / file_name,
        base / ".." / ".." / ".." / file_name,
        std::filesystem::path{file_name},
    };
    for (std::filesystem::path const &candidate : candidates) {
        std::error_code failure;
        if (std::filesystem::is_regular_file(candidate, failure)) {
            return candidate.string();
        }
    }
    return {};
}

} // namespace

struct audio_player::engine_state {
    ma_engine engine{};
    ma_sound sound{};
    bool engine_ready = false;
    bool sound_ready = false;
    double length_seconds = 0.0;
};

audio_player::audio_player() : state_{std::make_unique<engine_state>()}
{}

audio_player::~audio_player()
{
    stop_track();
}

bool audio_player::start_track(std::string const &file_name,
                               std::string &error_out)
{
    std::string const path = find_asset_path(file_name);
    if (path.empty()) {
        error_out = "fichier audio introuvable : " + file_name;
        return false;
    }
    if (ma_engine_init(nullptr, &state_->engine) != MA_SUCCESS) {
        error_out = "aucune sortie audio disponible";
        return false;
    }
    state_->engine_ready = true;
    if (ma_sound_init_from_file(&state_->engine, path.c_str(),
                                MA_SOUND_FLAG_DECODE, nullptr, nullptr,
                                &state_->sound) != MA_SUCCESS) {
        error_out = "decodage impossible : " + path;
        stop_track();
        return false;
    }
    state_->sound_ready = true;
    float length = 0.0f;
    if (ma_sound_get_length_in_seconds(&state_->sound, &length) == MA_SUCCESS) {
        state_->length_seconds = static_cast<double>(length);
    }
    if (ma_sound_start(&state_->sound) != MA_SUCCESS) {
        error_out = "demarrage de la lecture impossible";
        stop_track();
        return false;
    }
    return true;
}

void audio_player::stop_track()
{
    if (state_->sound_ready) {
        ma_sound_uninit(&state_->sound);
        state_->sound_ready = false;
    }
    if (state_->engine_ready) {
        ma_engine_uninit(&state_->engine);
        state_->engine_ready = false;
    }
}

double audio_player::get_track_length_seconds() const
{
    return state_->length_seconds;
}

} // namespace hypercom::client
