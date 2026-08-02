#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "common/protocol/dm_fetch_message.hpp"
#include "server/db/database_handle.hpp"

namespace hypercom::server {

// Boite aux lettres aveugle.
//
// Aucune methode ici ne peut lire le contenu d'un message : il n'existe nulle
// part sur cette machine de cle permettant de le faire. Un administrateur ne
// peut donc pas changer d'avis -- il n'y a rien a changer.
class dm_repository {
public:
    explicit dm_repository(database_handle &database);

    [[nodiscard]] bool insert_envelope(
        std::int64_t recipient_id,
        proto::wire_public_key const &sender_pubkey,
        std::span<std::uint8_t const> ciphertext, std::int64_t &out_id);

    [[nodiscard]] bool list_for_recipient(std::int64_t recipient_id,
                                          std::uint64_t since_id,
                                          std::uint16_t limit,
                                          proto::dm_list_response &out);

    // Acquitter, c'est supprimer. Ce qui n'existe plus sur le disque ne peut
    // pas etre saisi -- c'est la politique de retention des DM du projet.
    [[nodiscard]] bool delete_acknowledged(
        std::int64_t recipient_id, std::vector<std::uint64_t> const &ids);

    [[nodiscard]] bool count_pending(std::int64_t recipient_id,
                                     std::uint32_t &out);

private:
    database_handle &database_;
};

} // namespace hypercom::server
