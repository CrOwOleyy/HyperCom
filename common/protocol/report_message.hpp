#pragma once

#include <cstdint>
#include <string>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

namespace hypercom::proto {

// Signalement d'un dispositif legalement requis, pas d'un outil de
// moderation (BRIEF.md 13) : le serveur enregistre, il ne juge rien, et
// aucun des deux messages ne peut jamais porter le contenu d'un DM, qui reste
// illisible pour lui.
//
// La reponse est status_ok ou status_error, comme pour une suppression : il
// n'y a rien de plus a renvoyer qu'un accuse de reception.

struct report_post_request {
    std::uint64_t post_id = 0;
    std::string reason;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// target_pubkey, pas un handle : un pseudo peut changer de sens si le compte
// est recree, une cle publique jamais.
struct report_account_request {
    wire_public_key target_pubkey{};
    std::string reason;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
