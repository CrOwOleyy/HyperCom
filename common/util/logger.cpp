#include "common/util/logger.hpp"

#include <ctime>
#include <ostream>

#include "common/util/unix_clock.hpp"

namespace hypercom::util {
namespace {

constexpr std::string_view REDACTED_ADDRESS = "[redacted]";

void format_utc_timestamp(std::uint64_t seconds, char (&out)[32])
{
    std::time_t const raw = static_cast<std::time_t>(seconds);
    std::tm broken_down{};
#if defined(_WIN32)
    gmtime_s(&broken_down, &raw);
#else
    gmtime_r(&raw, &broken_down);
#endif
    std::strftime(out, sizeof(out), "%Y-%m-%dT%H:%M:%SZ", &broken_down);
}

} // namespace

logger::logger(log_level minimum, bool allow_peer_addresses,
               std::ostream &sink)
    : minimum_{minimum}, allow_peer_addresses_{allow_peer_addresses},
      sink_{sink}
{
}

bool logger::is_level_enabled(log_level level) const
{
    return level >= minimum_ && minimum_ != log_level::silent;
}

std::string_view logger::redact_peer_address(std::string_view address) const
{
    return allow_peer_addresses_ ? address : REDACTED_ADDRESS;
}

void logger::write_prefix(log_level level)
{
    char timestamp[32] = {};
    format_utc_timestamp(get_unix_timestamp(), timestamp);
    sink_ << timestamp << " [" << describe_log_level(level) << "] ";
}

void logger::write_entry(log_level level, std::string_view message)
{
    if (!is_level_enabled(level)) {
        return;
    }
    write_prefix(level);
    sink_ << message << '\n';
    // Purge systematique, y compris en info. Sur une sortie redirigee vers un
    // fichier, stdio met en tampon 4 KiB : sans ce flush, les lignes de
    // demarrage n'apparaissent qu'a l'arret du serveur. Le volume attendu ne
    // justifie aucune optimisation ici, et un journal en retard sur la realite
    // ne sert a personne.
    sink_.flush();
}

} // namespace hypercom::util
