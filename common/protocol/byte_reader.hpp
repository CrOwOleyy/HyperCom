#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "common/protocol/endian_codec.hpp"

namespace hypercom::proto {

// Lecteur borne. Tout ce qui vient du reseau passe par ici, sans exception.
//
// Trois choses a savoir avant de le modifier :
//   - il ne lit jamais au-dela du tampon, tout passe par take_slice ;
//   - il signale l'echec par la valeur de retour, et laisse la sortie intacte ;
//   - il ne fait aucune allocation avant d'avoir verifie la taille annoncee.
//
// Cette derniere regle est la plus importante. Si vous ajoutez une methode qui
// alloue, verifiez le plafond AVANT, pas apres.
class byte_reader {
public:
    explicit byte_reader(std::span<std::uint8_t const> buffer);

    template <typename T>
    [[nodiscard]] bool read_integer(T &out)
    {
        std::span<std::uint8_t const> slice;
        if (!take_slice(sizeof(T), slice)) {
            return false;
        }
        return load_little_endian(slice, out);
    }

    // Pour les champs de taille connue : cles publiques, signatures, nonces.
    [[nodiscard]] bool read_fixed_bytes(std::span<std::uint8_t> destination);

    // Lit [u32 taille][octets]. La taille est comparee a maximum_length et au
    // reste du tampon avant qu'on reserve quoi que ce soit.
    [[nodiscard]] bool read_length_prefixed(std::vector<std::uint8_t> &out,
                                            std::size_t maximum_length);

    [[nodiscard]] std::size_t count_remaining_bytes() const;

private:
    [[nodiscard]] bool take_slice(std::size_t size,
                                  std::span<std::uint8_t const> &out);

    std::span<std::uint8_t const> buffer_;
    std::size_t offset_;
};

} // namespace hypercom::proto
