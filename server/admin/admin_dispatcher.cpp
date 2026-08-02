#include "server/admin/admin_dispatcher.hpp"

#include "server/admin/admin_backup_command.hpp"
#include "server/admin/admin_motd_command.hpp"
#include "server/admin/admin_session_command.hpp"
#include "server/admin/admin_stats_command.hpp"

namespace hypercom::server {
namespace {

[[nodiscard]] std::string build_help()
{
    return "commandes disponibles\n"
           "  stats                      etat du serveur et volumetrie\n"
           "  sessions [list]            connexions en cours\n"
           "  sessions close <descr>     ferme une connexion\n"
           "  motd [show]                annonce en cours\n"
           "  motd set \"texte\"           publie une annonce\n"
           "  motd clear                 desactive l'annonce\n"
           "  backup <chemin>            sauvegarde a chaud de la base\n"
           "  help                       cette liste\n"
           "\n"
           "Non implemente : le rechargement a chaud de la configuration.\n"
           "Changer un listener demanderait de le reouvrir sous les connexions\n"
           "en cours ; tant que ce n'est pas traite proprement, il faut\n"
           "redemarrer le serveur.\n";
}

} // namespace

std::string execute_admin_command(admin_context &context,
                                  admin_command const &command,
                                  std::vector<int> &close_requests)
{
    if (command.verb == "stats") {
        return run_stats_command(context);
    }
    if (command.verb == "sessions") {
        return run_sessions_command(context, command, close_requests);
    }
    if (command.verb == "motd") {
        return run_motd_command(context, command);
    }
    if (command.verb == "backup") {
        return run_backup_command(context, command);
    }
    if (command.verb == "help") {
        return build_help();
    }
    return "commande inconnue : " + command.verb + "\nTaper help.\n";
}

} // namespace hypercom::server
