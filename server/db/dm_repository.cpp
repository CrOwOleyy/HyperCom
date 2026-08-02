#include "server/db/dm_repository.hpp"

#include <string>

#include "common/util/unix_clock.hpp"
#include "server/db/sql_binder.hpp"
#include "server/db/sql_column_reader.hpp"
#include "server/db/sql_statement.hpp"

namespace hypercom::server {

dm_repository::dm_repository(database_handle &database) : database_{database}
{
}

bool dm_repository::insert_envelope(
    std::int64_t recipient_id, proto::wire_public_key const &sender_pubkey,
    std::span<std::uint8_t const> ciphertext, std::int64_t &out_id)
{
    sql_statement statement{
        database_,
        // Aucun horodatage : la date d'envoi est a l'interieur du chiffre, que
        // le serveur ne peut pas ouvrir. Il ne tient donc pas de registre
        // horodate de qui echange avec qui.
        "INSERT INTO dm_envelopes (recipient_id, sender_pubkey, ciphertext) "
        "VALUES (?1, ?2, ?3)"};
    if (!bind_integer(statement, 1, recipient_id)
        || !bind_blob(statement, 2, sender_pubkey)
        || !bind_blob(statement, 3, ciphertext)) {
        return false;
    }
    if (statement.step_row() != step_result::done) {
        return false;
    }
    out_id = database_.get_last_insert_id();
    return true;
}

bool dm_repository::list_for_recipient(std::int64_t recipient_id,
                                       std::uint64_t since_id,
                                       std::uint16_t limit,
                                       proto::dm_list_response &out)
{
    sql_statement statement{
        database_,
        // Ordonne par id, pas par date : le serveur ne stocke plus d'horodatage
        // et l'id auto-incremente donne deja l'ordre d'arrivee.
        "SELECT id, sender_pubkey, ciphertext "
        "FROM dm_envelopes WHERE recipient_id = ?1 AND id > ?2 "
        "ORDER BY id LIMIT ?3"};
    if (!bind_integer(statement, 1, recipient_id)
        || !bind_integer(statement, 2, static_cast<std::int64_t>(since_id))
        || !bind_integer(statement, 3, static_cast<std::int64_t>(limit) + 1)) {
        return false;
    }
    out.envelopes.clear();
    out.has_more = 0;
    while (statement.step_row() == step_result::row) {
        if (out.envelopes.size() >= limit) {
            out.has_more = 1;
            break;
        }
        proto::dm_envelope_record record;
        record.id = static_cast<std::uint64_t>(read_integer(statement, 0));
        if (!read_fixed_bytes(statement, 1, record.sender_pubkey)) {
            return false;
        }
        record.ciphertext = read_blob(statement, 2);
        out.envelopes.push_back(std::move(record));
    }
    return true;
}

bool dm_repository::delete_acknowledged(std::int64_t recipient_id,
                                        std::vector<std::uint64_t> const &ids)
{
    // Une requete preparee reutilisee, jamais une liste IN construite par
    // concatenation. C'est plus verbeux et ca ne s'injecte pas.
    std::string error;
    if (!database_.execute_script("BEGIN IMMEDIATE", error)) {
        return false;
    }
    sql_statement statement{
        database_,
        "DELETE FROM dm_envelopes WHERE id = ?1 AND recipient_id = ?2"};
    bool succeeded = true;
    for (std::uint64_t const identifier : ids) {
        if (!succeeded) {
            break;
        }
        // recipient_id est dans la clause WHERE : personne ne peut supprimer
        // les messages de quelqu'un d'autre en devinant des identifiants.
        succeeded = statement.reset_for_reuse()
                    && bind_integer(statement, 1,
                                    static_cast<std::int64_t>(identifier))
                    && bind_integer(statement, 2, recipient_id)
                    && statement.step_row() == step_result::done;
    }
    return database_.execute_script(succeeded ? "COMMIT" : "ROLLBACK", error)
           && succeeded;
}

bool dm_repository::count_pending(std::int64_t recipient_id,
                                  std::uint32_t &out)
{
    sql_statement statement{
        database_, "SELECT COUNT(*) FROM dm_envelopes WHERE recipient_id = ?1"};
    if (!bind_integer(statement, 1, recipient_id)
        || statement.step_row() != step_result::row) {
        return false;
    }
    out = static_cast<std::uint32_t>(read_integer(statement, 0));
    return true;
}

} // namespace hypercom::server
