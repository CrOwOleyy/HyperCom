#include "client/ui/server_actions.hpp"

#include "client/cli/invite_link.hpp"
#include "client/keystore/identity_store.hpp"
#include "client/keystore/master_seed_store.hpp"
#include "client/keystore/server_identity.hpp"
#include "client/ui/ui_actions.hpp"
#include "common/crypto/secure_memory.hpp"
#include "common/util/hex_codec.hpp"

#include <algorithm>

namespace hypercom::client {
namespace {

[[nodiscard]] bool resolve_identity(server_entry const &entry,
                                    app_state const &app,
                                    crypto::identity_keypair &out,
                                    std::string &error_out)
{
    if (entry.source == identity_source::imported) {
        identity_store store{entry.imported_identity_path};
        return store.unlock_identity(app.passphrase, out, error_out);
    }
    master_seed_store seeds{app.master_seed_path};
    crypto::ed25519_seed seed{};
    bool ready = seeds.has_stored_seed()
                     ? seeds.unlock_seed(app.passphrase, seed, error_out)
                     : seeds.create_seed(app.passphrase, seed, error_out);
    if (ready) {
        ready = derive_server_identity(seed, entry.server_key, out);
        if (!ready) {
            error_out = "derivation de l'identite impossible";
        }
    }
    crypto::wipe_bytes(seed);
    return ready;
}

// Copies the slots' current state back into the registry, then
// reseals the whole thing.
[[nodiscard]] bool persist(app_state const &app, server_slot_list const &slots,
                           std::string &error_out)
{
    std::vector<server_entry> entries;
    entries.reserve(slots.size());
    for (std::unique_ptr<server_slot> const &slot : slots) {
        entries.push_back(slot->entry);
    }
    server_registry registry{app.registry_path};
    return registry.save(app.passphrase, entries, error_out);
}

void report(app_state &app, std::string message, bool is_error)
{
    app.status_message = std::move(message);
    app.status_is_error = is_error;
}

} // namespace

bool build_slot(server_entry const &entry, app_state const &app,
                std::unique_ptr<server_slot> &out, std::string &error_out)
{
    auto slot = std::make_unique<server_slot>();
    slot->entry = entry;
    if (!resolve_identity(entry, app, slot->identity, error_out)) {
        return false;
    }
    slot->connection = std::make_unique<server_connection>(entry.server_key);
    slot->session =
        std::make_unique<client_session>(*slot->connection, slot->identity);
    slot->view.endpoint = entry.endpoint;
    slot->view.current_lang = app.current_lang;
    util::encode_hex(slot->identity.get_public_key(), slot->view.identity_hex);
    util::encode_hex(entry.server_key, slot->view.server_key_hex);
    out = std::move(slot);
    return true;
}

bool connect_slot(server_slot &slot, std::string &error_out)
{
    if (!slot.entry.trust_acknowledged) {
        error_out = "avertissement non acquitte pour " + slot.entry.label;
        return false;
    }
    if (slot.connection->is_open()) {
        slot.view.connected = true;
        return true;
    }
    if (!slot.connection->open_session(slot.entry.endpoint, error_out) ||
        !slot.session->authenticate(error_out)) {
        slot.view.connected = false;
        return false;
    }
    slot.view.connected = true;
    slot.view.registered = !slot.session->needs_registration();
    if (slot.view.registered) {
        slot.view.handle = slot.session->get_handle();
    }
    return true;
}

void add_server_from_invite(app_state &app, server_slot_list &slots)
{
    invite_link link;
    std::string failure;
    if (!parse_invite_link(app.invite_input, link, failure)) {
        report(app, failure, true);
        return;
    }
    server_entry entry;
    entry.label = link.host;
    entry.endpoint = {link.host, link.port, {}, 0};
    std::vector<std::uint8_t> decoded;
    if (!util::decode_hex(link.server_key_hex, decoded) ||
        decoded.size() != entry.server_key.size()) {
        report(app, "cle du serveur illisible", true);
        return;
    }
    std::copy(decoded.begin(), decoded.end(), entry.server_key.begin());
    auto const clash = std::find_if(
        slots.begin(), slots.end(), [&entry](auto const &existing) {
            return existing->entry.server_key == entry.server_key;
        });
    if (clash != slots.end()) {
        report(app, "ce serveur est deja dans la liste", true);
        return;
    }
    std::unique_ptr<server_slot> slot;
    if (!build_slot(entry, app, slot, failure)) {
        report(app, failure, true);
        return;
    }
    slots.push_back(std::move(slot));
    if (!persist(app, slots, failure)) {
        report(app, failure, true);
        return;
    }
    app.invite_input[0] = '\0';
    report(app, "serveur ajoute : " + entry.label, false);
}

void acknowledge_slot_trust(app_state &app, server_slot_list &slots,
                            std::size_t index)
{
    if (index >= slots.size()) {
        return;
    }
    slots[index]->entry.trust_acknowledged = true;
    std::string failure;
    if (!persist(app, slots, failure)) {
        report(app, failure, true);
    }
}

} // namespace hypercom::client
