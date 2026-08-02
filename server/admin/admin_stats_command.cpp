#include "server/admin/admin_stats_command.hpp"

#include "common/util/unix_clock.hpp"
#include "server/admin/admin_text_format.hpp"
#include "server/db/stats_repository.hpp"

namespace hypercom::server {
namespace {

[[nodiscard]] std::uint32_t count_authenticated(
    connection_registry const &registry)
{
    std::uint32_t total = 0;
    for (auto const &entry : registry.connections) {
        if (entry.second->session.phase == session_phase::authenticated) {
            ++total;
        }
    }
    return total;
}

[[nodiscard]] std::string format_line(std::string_view label,
                                      std::string const &value)
{
    return "  " + pad_right(label, 22) + ": " + value + "\n";
}

} // namespace

std::string run_stats_command(admin_context &context)
{
    std::uint64_t const now = util::get_unix_timestamp();
    std::string text = "hypercom -- statistiques\n";
    text += format_line("en service depuis",
                        format_duration(now - context.started_at));
    text += format_line(
        "connexions",
        std::to_string(context.registry.connections.size()) + " / "
            + std::to_string(context.config.limits.max_connections));
    text += format_line("dont authentifiees",
                        std::to_string(count_authenticated(context.registry)));
    text += format_line("inscriptions",
                        context.config.registration_open ? "ouvertes"
                                                         : "fermees");
    text += format_line("journalisation des IP",
                        context.config.logging.log_peer_addresses
                            ? "ACTIVE"
                            : "desactivee (defaut)");
    server_counts counts;
    stats_repository repository{context.database};
    if (!repository.collect_counts(counts)) {
        return text + "  (compteurs de base illisibles)\n";
    }
    text += format_line("comptes", std::to_string(counts.users));
    text += format_line("forums", std::to_string(counts.forums));
    text += format_line("posts", std::to_string(counts.posts));
    text += format_line("commentaires", std::to_string(counts.comments));
    text += format_line("enveloppes en attente",
                        std::to_string(counts.pending_envelopes));
    return text;
}

} // namespace hypercom::server
