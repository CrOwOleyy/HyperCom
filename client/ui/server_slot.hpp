#pragma once

#include "client/cli/cli_context.hpp"
#include "client/keystore/server_registry.hpp"
#include "client/net/client_session.hpp"
#include "client/net/server_connection.hpp"
#include "client/ui/ui_state.hpp"
#include "common/crypto/identity_keypair.hpp"

#include <memory>
#include <vector>

namespace hypercom::client {

// Everything that belongs to ONE server: its connection, its own
// identity, its session, and the displayable state of its content.
//
// The identity is different on each server (see server_identity.hpp),
// so nothing here is shareable between slots -- that's precisely the
// point.
struct server_slot {
    server_entry entry;
    crypto::identity_keypair identity;
    std::unique_ptr<server_connection> connection;
    std::unique_ptr<client_session> session;
    ui_state view;
};

// Slots are stored as unique_ptr, never by value in a vector.
//
// client_session holds server_connection and identity_keypair by
// REFERENCE: if the container reallocated, those references would
// point into thin air. The unique_ptr pins the address of the whole
// slot, identity included. This one bit hard once, as a crash that only
// showed up after adding a third or fourth server -- exactly the kind
// of bug that vanishes the moment you attach a debugger to look at it.
using server_slot_list = std::vector<std::unique_ptr<server_slot>>;

// Builds the context expected by all the existing drawing and action
// functions. It holds only references: building it fresh every frame
// costs nothing, and avoids having to touch the forty-seven
// signatures that take it as a parameter.
[[nodiscard]] inline cli_context make_context(server_slot &slot)
{
    return cli_context{*slot.connection, slot.identity, *slot.session};
}

} // namespace hypercom::client
