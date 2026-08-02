#pragma once

#include <cstdint>
#include <vector>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"

namespace hypercom::proto {

// Listes : [u16 nombre][elements...]
//
// Un seul endroit ou le plafond d'une liste est verifie. Recopier la boucle
// dans chaque message finit toujours par produire une variante ou le reserve()
// passe avant le controle.
//
// Noter qu'on ne reserve rien d'avance : un pair qui annonce 65535 elements
// n'obtient aucune allocation tant qu'il n'a pas fourni les octets.

template <typename record_type>
[[nodiscard]] bool read_record_list(byte_reader &reader,
                                    std::vector<record_type> &out,
                                    std::uint16_t maximum_items)
{
    std::uint16_t announced_count = 0;
    if (!reader.read_integer(announced_count)) {
        return false;
    }
    if (announced_count > maximum_items) {
        return false;
    }
    std::vector<record_type> decoded;
    for (std::uint16_t index = 0; index < announced_count; ++index) {
        record_type item;
        if (!item.read_from(reader)) {
            return false;
        }
        decoded.push_back(std::move(item));
    }
    out = std::move(decoded);
    return true;
}

template <typename record_type>
void write_record_list(byte_writer &writer,
                       std::vector<record_type> const &items)
{
    writer.write_integer(static_cast<std::uint16_t>(items.size()));
    for (record_type const &item : items) {
        item.write_to(writer);
    }
}

} // namespace hypercom::proto
