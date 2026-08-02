#pragma once

#include <cstddef>
#include <string>
#include <string_view>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"

namespace hypercom::proto {

// Champ texte : [u32 taille][octets UTF-8].
//
// Le meme prefixe u32 sert aux textes et aux blobs. Deux octets de plus par
// chaine, mais un seul chemin de decodage a auditer et a fuzzer.

// Ce que le serveur accepte, il le resservira tel quel a d'autres clients.
// D'ou le refus de :
//   - l'UTF-8 invalide, sur-long, ou hors plan Unicode
//   - les demi-codets de substitution (U+D800..U+DFFF)
//   - les controles C0, sauf tabulation et saut de ligne
//   - le retour chariot, pour rester coherent avec la regle G6
[[nodiscard]] bool validate_text_field(std::string_view text);

// Lit, borne, valide. N'ecrit la sortie que si les trois reussissent.
[[nodiscard]] bool read_text_field(byte_reader &reader, std::string &out,
                                   std::size_t maximum_length);

void write_text_field(byte_writer &writer, std::string_view text);

} // namespace hypercom::proto
