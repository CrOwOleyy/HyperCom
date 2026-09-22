#include "server/admin/admin_ban_command.hpp"

#include <vector>

#include "common/protocol/wire_key.hpp"
#include "common/util/hex_codec.hpp"
#include "server/db/user_repository.hpp"

namespace hypercom::server {
namespace {

[[nodiscard]] bool decode_pubkey(std::string const &text,
                                 proto::wire_public_key &out)
{
    std::vector<std::uint8_t> decoded;
    if (!util::decode_hex(text, decoded) || decoded.size() != out.size()) {
        return false;
    }
    std::copy(decoded.begin(), decoded.end(), out.begin());
    return true;
}

} // namespace

std::string run_ban_command(admin_context &context,
                            admin_command const &command, bool banned)
{
    if (command.arguments.empty()) {
        return std::string{"usage : "} + (banned ? "ban" : "unban")
               + " <cle_publique_hex>\n";
    }
    proto::wire_public_key pubkey{};
    if (!decode_pubkey(command.arguments.front(), pubkey)) {
        return "cle publique invalide : 64 caracteres hexadecimaux "
               "attendus\n";
    }
    user_repository users{context.database};
    user_row target;
    if (!users.find_by_pubkey(pubkey, target)) {
        return "aucun compte pour cette cle\n";
    }
    if (!users.set_banned(target.id, banned)) {
        return "echec : ecriture en base impossible\n";
    }
    context.logger.write_entry(
        util::log_level::info,
        std::string{banned ? "compte banni" : "compte reintegre"}
            + " par l'administration");
    return "@" + target.handle + (banned ? " banni\n" : " reintegre\n");
}

} // namespace hypercom::server
