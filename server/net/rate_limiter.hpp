#pragma once

#include <cstdint>

namespace hypercom::server {

// Fenetre glissante grossiere, une minute.
//
// L'objet ne detient AUCUN etat : il ne porte que le plafond, et les compteurs
// lui sont passes par reference. C'est impose par G4.
//
// Ou vivent ces compteurs est la vraie question, et elle a deja ete tranchee
// de travers une fois : les mettre dans la session revient a les remettre a
// zero a chaque nouvelle connexion, donc a supprimer la limite. Ils vivent
// desormais dans rate_tracker, partages entre connexions -- lire le compromis
// documente dans rate_tracker.hpp avant de revenir la-dessus.
class rate_limiter {
public:
    explicit rate_limiter(std::uint32_t max_events_per_minute);

    [[nodiscard]] bool register_event(std::uint64_t now,
                                      std::uint32_t &counter,
                                      std::uint64_t &window_start) const;

private:
    std::uint32_t max_events_per_minute_;
};

} // namespace hypercom::server
