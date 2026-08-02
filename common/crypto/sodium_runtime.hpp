#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace hypercom::crypto {

// A appeler une fois au demarrage de tout processus qui touche a la crypto,
// AVANT toute autre fonction de ce namespace. sodium_init() choisit les
// implementations selon le processeur et amorce le generateur aleatoire ;
// l'oublier rend le reste indefini.
//
// Renvoie false si l'initialisation echoue. Dans ce cas le programme doit
// s'arreter : il n'existe aucun mode degrade acceptable.
[[nodiscard]] bool initialize_sodium();

// Aleatoire cryptographique. Unique source du projet -- ni std::random_device,
// ni rand(), ni mt19937 ne doivent apparaitre dans du code de securite.
void fill_random_bytes(std::span<std::uint8_t> destination);

} // namespace hypercom::crypto
