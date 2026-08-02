#include "server/admin/admin_motd_command.hpp"

#include "common/protocol/protocol_limits.hpp"
#include "common/protocol/text_field_codec.hpp"
#include "server/db/motd_repository.hpp"

namespace hypercom::server {
namespace {

[[nodiscard]] std::string show_motd(admin_context &context)
{
    motd_repository repository{context.database};
    proto::motd_push active;
    if (!repository.find_active_motd(active)) {
        return "aucune annonce active\n";
    }
    return "revision " + std::to_string(active.revision) + "\n" + active.body
           + "\n";
}

[[nodiscard]] std::string set_motd(admin_context &context,
                                   std::string const &body)
{
    // Meme validation que pour un message venu du reseau : l'annonce sera
    // resservie a tous les clients, et un administrateur distrait n'a pas plus
    // le droit qu'un inconnu d'y glisser des octets de controle.
    if (body.size() > proto::MAX_MOTD_LENGTH) {
        return "refus : annonce trop longue (max "
               + std::to_string(proto::MAX_MOTD_LENGTH) + " octets)\n";
    }
    if (!proto::validate_text_field(body)) {
        return "refus : l'annonce doit etre de l'UTF-8 valide, sans "
               "caracteres de controle\n";
    }
    motd_repository repository{context.database};
    if (!repository.publish_motd(body)) {
        return "echec : ecriture en base impossible\n";
    }
    context.logger.write_entry(util::log_level::info,
                               "MOTD modifie par l'administration");
    return "annonce publiee, active des la prochaine connexion\n";
}

[[nodiscard]] std::string clear_motd(admin_context &context)
{
    motd_repository repository{context.database};
    if (!repository.clear_active_motd()) {
        return "echec : ecriture en base impossible\n";
    }
    context.logger.write_entry(util::log_level::info,
                               "MOTD desactive par l'administration");
    return "annonce desactivee\n";
}

} // namespace

std::string run_motd_command(admin_context &context,
                             admin_command const &command)
{
    if (command.arguments.empty() || command.arguments.front() == "show") {
        return show_motd(context);
    }
    std::string const &action = command.arguments.front();
    if (action == "clear") {
        return clear_motd(context);
    }
    if (action == "set") {
        if (command.arguments.size() < 2) {
            return "usage : motd set \"texte de l'annonce\"\n";
        }
        return set_motd(context, command.arguments[1]);
    }
    return "usage : motd [show | set \"texte\" | clear]\n";
}

} // namespace hypercom::server
