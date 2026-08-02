#include "server/admin/admin_backup_command.hpp"

#include "server/db/database_backup.hpp"

namespace hypercom::server {

std::string run_backup_command(admin_context &context,
                               admin_command const &command)
{
    if (command.arguments.empty()) {
        return "usage : backup <chemin/de/sortie.db>\n";
    }
    std::string const &destination = command.arguments.front();
    std::string failure;
    if (!backup_database_to(context.database, destination, failure)) {
        return "echec : " + failure + "\n";
    }
    context.logger.write_entry(util::log_level::info,
                               "sauvegarde a chaud effectuee");
    // Rappel utile : la base seule ne suffit pas a redemarrer un serveur
    // identique. Perdre la cle statique casse l'epinglage de tous les clients.
    return "sauvegarde ecrite : " + destination
           + "\nPenser a sauvegarder aussi " + context.config.paths.server_key_path
           + " : sans elle, les clients refuseront de se reconnecter.\n";
}

} // namespace hypercom::server
