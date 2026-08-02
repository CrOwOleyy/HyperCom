#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "common/protocol/message_type.hpp"

namespace hypercom::proto {

// Cadrage : [u32 body_size][u8 type][payload]
//
// body_size compte l'octet de type et le payload, mais pas le champ de
// longueur lui-meme. Une trame valide verifie donc 1 <= body_size <=
// MAX_BODY_SIZE. Le piege classique est de croire que body_size ne couvre que
// le payload -- relisez encode_frame en cas de doute.
struct frame_header {
    std::uint32_t body_size;
    message_type type;
};

// Ne lit que le champ de longueur. Le lecteur de flux s'en sert pour savoir
// combien d'octets attendre avant de reserver quoi que ce soit.
[[nodiscard]] bool peek_body_size(std::span<std::uint8_t const> input,
                                  std::uint32_t &out);

// Entete complet : longueur bornee et type reconnu. Un type inconnu echoue
// ici, ce qui evite qu'il atteigne un handler.
[[nodiscard]] bool decode_frame_header(std::span<std::uint8_t const> input,
                                       frame_header &out);

// Echoue si le payload depasse le plafond, ce qui indique un bug d'appelant
// plutot qu'une donnee hostile.
[[nodiscard]] bool encode_frame(message_type type,
                                std::span<std::uint8_t const> payload,
                                std::vector<std::uint8_t> &out);

} // namespace hypercom::proto
