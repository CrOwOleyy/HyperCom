#pragma once

#include <cstdint>
#include <string>

#include "common/protocol/wire_key.hpp"

namespace hypercom::server {

enum class session_phase {
    awaiting_handshake,
    awaiting_hello,
    awaiting_auth,
    authenticated,
};

// Etat applicatif d'une connexion.
//
// peer_address est en memoire pour le comptage par adresse, et n'atteint jamais
// un journal sans passer par logger::redact_peer_address. Elle n'est ecrite
// nulle part sur disque.
//
// Les compteurs de debit vivent ici plutot que dans une table globale : c'est
// impose par G4, et ca a un effet secondaire heureux -- ils disparaissent avec
// la connexion, sans laisser d'historique par identite.
struct session_state {
    session_phase phase = session_phase::awaiting_handshake;
    proto::wire_public_key announced_pubkey{};
    proto::wire_nonce challenge_nonce{};
    std::int64_t user_id = 0;
    std::string handle;
    std::string peer_address;
    std::uint64_t connected_at = 0;
    std::uint64_t last_activity_at = 0;
    std::uint32_t requests_in_window = 0;
    std::uint64_t rate_window_start = 0;
};

} // namespace hypercom::server
