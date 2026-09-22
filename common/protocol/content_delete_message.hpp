#pragma once

#include <cstdint>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"

namespace hypercom::proto {

// Retrait de son propre contenu. La cible n'est designee que par son
// identifiant : l'auteur n'est PAS un champ du message, il vient de la session
// authentifiee. Un expediteur ne peut donc pas se declarer auteur de ce qu'il
// n'a pas ecrit.
//
// La reponse est status_ok ou status_error, sans structure dediee : il n'y a
// rien a renvoyer qu'un acquittement.

struct post_delete_request {
    std::uint64_t post_id = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

struct comment_delete_request {
    std::uint64_t comment_id = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
