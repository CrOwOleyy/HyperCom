#include "server/admin/admin_reports_command.hpp"

#include "common/util/hex_codec.hpp"
#include "server/admin/admin_text_format.hpp"
#include "server/db/post_repository.hpp"
#include "server/db/report_repository.hpp"

#include <charconv>

namespace hypercom::server {
namespace {

constexpr std::uint16_t REPORTS_LIST_LIMIT = 200;

[[nodiscard]] std::string list_reports(admin_context &context)
{
    report_repository reports{context.database};
    std::vector<report_row> rows;
    if (!reports.list_reports(REPORTS_LIST_LIMIT, rows)) {
        return "echec : lecture des signalements impossible\n";
    }
    if (rows.empty()) {
        return "(aucun signalement)\n";
    }
    std::string text = pad_right("ID", 6) + pad_right("TYPE", 10) +
                       pad_right("CIBLE", 20) + pad_right("MOTIF", 30) +
                       "PAR\n";
    for (report_row const &row : rows) {
        std::string target = std::to_string(row.post_id);
        if (row.kind != "post") {
            util::encode_hex(row.target_pubkey, target);
            target = target.substr(0, 16);
        }
        text += pad_right(std::to_string(row.id), 6);
        text += pad_right(row.kind, 10);
        text += pad_right(target, 20);
        text += pad_right(row.reason.substr(0, 28), 30);
        text += "#" + std::to_string(row.reporter_id);
        text.push_back('\n');
    }
    return text;
}

[[nodiscard]] bool parse_report_id(std::string const &text, std::int64_t &out)
{
    auto const result =
        std::from_chars(text.data(), text.data() + text.size(), out);
    return result.ec == std::errc{};
}

[[nodiscard]] std::string clear_report(admin_context &context,
                                       std::string const &argument)
{
    std::int64_t id = 0;
    if (!parse_report_id(argument, id)) {
        return "usage : reports clear <id>\n";
    }
    report_repository reports{context.database};
    if (!reports.clear_report(id)) {
        return "echec : signalement introuvable ou deja classe\n";
    }
    context.logger.write_entry(util::log_level::info,
                               "signalement classe par l'administration");
    return "signalement " + argument + " classe\n";
}

[[nodiscard]] std::string delete_reported_post(admin_context &context,
                                               std::string const &argument)
{
    std::int64_t post_id = 0;
    if (!parse_report_id(argument, post_id)) {
        return "usage : reports delete-post <post_id>\n";
    }
    post_repository posts{context.database};
    if (!posts.admin_delete_post(post_id)) {
        return "echec : post introuvable ou deja supprime\n";
    }
    // Logged without the reason or the post's text: the trace says an
    // action took place, not what motivated the original report.
    context.logger.write_entry(
        util::log_level::info,
        "post supprime par l'administration suite a un signalement");
    return "post " + argument + " supprime\n";
}

} // namespace

std::string run_reports_command(admin_context &context,
                                admin_command const &command)
{
    if (command.arguments.empty() || command.arguments.front() == "list") {
        return list_reports(context);
    }
    std::string const &action = command.arguments.front();
    if (action == "clear" && command.arguments.size() >= 2) {
        return clear_report(context, command.arguments[1]);
    }
    if (action == "delete-post" && command.arguments.size() >= 2) {
        return delete_reported_post(context, command.arguments[1]);
    }
    return "usage : reports [list | clear <id> | delete-post <post_id>]\n";
}

} // namespace hypercom::server
