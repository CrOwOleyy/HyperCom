#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace hypercom::proto {

// Extraction de messages sur un flux TCP, partagee par le client et le serveur.
//
// TCP est un flux d'octets : une lecture peut rendre un demi-message, ou trois
// messages colles. Cette fonction est le seul endroit du projet qui gere ce
// decoupage, et elle applique la regle du brief : la longueur annoncee est
// comparee au plafond AVANT toute reservation memoire.
//
// Renvoie true quand un message complet a ete detache de buffer. Renvoie false
// avec malformed = true si la longueur annoncee viole le plafond -- la
// connexion doit alors etre fermee, jamais reprise : un flux desynchronise ne
// se rattrape pas.
[[nodiscard]] bool extract_length_prefixed_message(
    std::vector<std::uint8_t> &buffer, std::size_t maximum_size,
    std::vector<std::uint8_t> &out, bool &malformed);

void append_length_prefixed_message(std::span<std::uint8_t const> payload,
                                    std::vector<std::uint8_t> &out);

} // namespace hypercom::proto
