#pragma once

#include <cstdint>

namespace hypercom::server {

// Fenetre glissante grossiere, une minute.
//
// L'objet ne detient AUCUN etat : les compteurs sont passes par reference et
// vivent dans la session appelante. C'est impose par G4, et ca evite au
// passage la table globale « adresse -> historique » qu'un limiteur classique
// maintiendrait -- table qui serait, elle, un journal d'IP deguise.
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
