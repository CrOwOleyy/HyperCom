#pragma once

#include <memory>
#include <string>

namespace hypercom::client {

// Lecture du theme d'accueil.
//
// miniaudio est la seule dependance du projet hors libsodium, SQLite et ImGui,
// et elle est cantonnee au client graphique : ni le serveur, ni le client CLI,
// ni la bibliotheque commune ne la voient passer.
//
// Aucun echec n'est fatal. Machine sans carte son, serveur audio absent,
// peripherique deja occupe, fichier introuvable : dans tous ces cas le client
// s'ouvre normalement, en silence. La musique est un agrement, pas une
// condition de fonctionnement.
class audio_player {
public:
    audio_player();

    ~audio_player();

    // file_name est cherche a cote de l'executable puis dans les repertoires
    // parents, ce qui couvre aussi bien un binaire installe qu'un build local.
    [[nodiscard]] bool start_track(std::string const &file_name,
                                   std::string &error_out);

    void stop_track();

    // 0 si le morceau n'a pas pu etre ouvert. L'appelant retombe alors sur une
    // duree fixe plutot que de jouer une animation de duree nulle.
    [[nodiscard]] double get_track_length_seconds() const;

private:
    struct engine_state;

    std::unique_ptr<engine_state> state_;
};

} // namespace hypercom::client
