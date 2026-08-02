#include "common/protocol/error_code.hpp"

namespace hypercom::proto {

std::string_view describe_error_code(error_code code)
{
    switch (code) {
        case error_code::none:                  return "ok";
        case error_code::malformed_frame:       return "trame invalide";
        case error_code::unsupported_version:   return "version non supportee";
        case error_code::not_authenticated:     return "authentification requise";
        case error_code::already_authenticated: return "session deja authentifiee";
        case error_code::authentication_failed: return "authentification refusee";
        case error_code::handle_unavailable:    return "pseudo indisponible";
        case error_code::invalid_field:         return "champ invalide";
        case error_code::not_found:             return "introuvable";
        case error_code::permission_denied:     return "acces refuse";
        case error_code::rate_limited:          return "trop de requetes";
        case error_code::payload_too_large:     return "charge utile trop grande";
        case error_code::duplicate_entry:       return "entree deja existante";
        case error_code::internal_error:        return "erreur interne";
        case error_code::not_implemented:       return "non implemente";
    }
    return "erreur inconnue";
}

} // namespace hypercom::proto
