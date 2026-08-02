#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "common/crypto/identity_keypair.hpp"
#include "common/protocol/prekey_fetch_message.hpp"

namespace hypercom::client {

// Chiffrement de bout en bout des messages prives, cote client uniquement.
//
// Rien de ce fichier n'existe cote serveur, et c'est tout l'interet : il n'y a
// la-bas ni code, ni cle, ni chemin permettant d'ouvrir une enveloppe.

// La prekey est DERIVEE de l'identite plutot que stockee : HKDF de la graine
// Ed25519. Consequence pratique -- il n'y a qu'un seul secret a sauvegarder, et
// la prekey se retrouve a l'identique sur n'importe quelle machine ou la cle
// est restauree.
//
// Contrepartie assumee en v1 : cette prekey ne tourne pas. La confidentialite
// persistante repose donc entierement sur la cle ephemere, regeneree a chaque
// message. La rotation de prekey est un ajout v2, prevu sans changement de
// format d'enveloppe.
[[nodiscard]] bool derive_local_prekey(
    crypto::identity_keypair const &identity,
    crypto::x25519_public_key &public_out,
    crypto::x25519_secret_key &secret_out);

// A appeler AVANT d'utiliser une prekey servie par le serveur. Un serveur qui
// substituerait sa propre prekey pour s'interposer echouerait ici : il ne
// possede pas la cle d'identite de la personne visee.
[[nodiscard]] bool verify_prekey_bundle(
    proto::prekey_bundle_response const &bundle);

[[nodiscard]] bool seal_direct_message(
    crypto::identity_keypair const &sender,
    proto::prekey_bundle_response const &recipient_bundle,
    std::string_view text, std::vector<std::uint8_t> &out,
    std::string &error_out);

// sent_at_out vient de l'INTERIEUR du chiffre : le serveur ne connait pas la
// date d'envoi, seul le destinataire la retrouve en dechiffrant.
[[nodiscard]] bool open_direct_message(
    crypto::identity_keypair const &recipient,
    std::span<std::uint8_t const> envelope, std::string &text_out,
    std::uint64_t &sent_at_out, std::string &error_out);

} // namespace hypercom::client
