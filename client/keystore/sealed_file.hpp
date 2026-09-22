#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace hypercom::client {

// Lecture et ecriture des fichiers de secrets du client, en 0600.
//
// Partage par la graine maitresse et le registre des serveurs : les deux
// ecrivent un blob scelle et rien d'autre, il n'y a aucune raison d'en avoir
// deux implementations qui divergeraient avec le temps.

// Sous POSIX, un chmod qui echoue est une erreur remontee, pas un
// avertissement avale : un fichier de secrets lisible par tout le monde ne doit
// pas passer inapercu.
[[nodiscard]] bool write_sealed_file(std::string const &path,
                                     std::span<std::uint8_t const> sealed,
                                     std::string &error_out);

[[nodiscard]] bool read_sealed_file(std::string const &path,
                                    std::vector<std::uint8_t> &out);

} // namespace hypercom::client
