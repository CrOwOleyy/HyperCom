#pragma once

#include <cstdint>
#include <iosfwd>
#include <string_view>

#include "common/util/log_level.hpp"

namespace hypercom::util {

// Journalisation explicite : aucune instance globale, aucun singleton (G4).
// Le logger se passe en parametre a tout ce qui doit journaliser.
//
// Le logger porte aussi la politique d'adresses de pairs, et c'est la un choix
// de conception plus qu'un detail : par defaut elle est a false, donc aucune IP
// n'est journalisee. Les appelants n'ont pas a y penser -- ils passent par
// redact_peer_address(), qui rend une chaine neutre tant que hypercom.conf
// n'a pas explicitement ouvert la politique.
class logger {
public:
    logger(log_level minimum, bool allow_peer_addresses, std::ostream &sink);

    void write_entry(log_level level, std::string_view message);

    [[nodiscard]] bool is_level_enabled(log_level level) const;

    // Rend l'adresse telle quelle si et seulement si la politique l'autorise,
    // et "[redacted]" sinon. C'est le seul chemin autorise vers un journal.
    [[nodiscard]] std::string_view
    redact_peer_address(std::string_view address) const;

private:
    void write_prefix(log_level level);

    log_level minimum_;
    bool allow_peer_addresses_;
    std::ostream &sink_;
};

} // namespace hypercom::util
