#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "common/protocol/endian_codec.hpp"

namespace hypercom::proto {

// Pendant du byte_reader, en ecriture.
//
// Il ne verifie aucun plafond, contrairement au lecteur. La raison : il ne
// serialise que des donnees qu'on a construites et deja validees. Le seul
// controle de taille se fait dans encode_frame, qui refuse un payload trop
// gros. Si vous vous retrouvez a ecrire ici des donnees venues du reseau,
// c'est probablement le signe qu'il faut revoir le decoupage.
class byte_writer {
public:
    explicit byte_writer(std::vector<std::uint8_t> &target);

    template <typename T>
    void write_integer(T value)
    {
        std::size_t const offset = target_.size();
        target_.resize(offset + sizeof(T));
        // La place fait exactement sizeof(T), l'ecriture ne peut pas echouer.
        static_cast<void>(store_little_endian(
            value, std::span<std::uint8_t>{target_}.subspan(offset)));
    }

    void write_fixed_bytes(std::span<std::uint8_t const> data);

    void write_length_prefixed(std::span<std::uint8_t const> data);

    [[nodiscard]] std::size_t count_written_bytes() const;

private:
    std::vector<std::uint8_t> &target_;
    std::size_t initial_size_;
};

} // namespace hypercom::proto
