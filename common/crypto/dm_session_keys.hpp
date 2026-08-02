#pragma once

#include "common/crypto/identity_keypair.hpp"
#include "common/crypto/key_types.hpp"

namespace hypercom::crypto {

// X3DH simplifie (BRIEF.md 6). Trois echanges Diffie-Hellman, un seul secret :
//
//   DH1 = DH(IK_expediteur, SPK_destinataire)   authentifie l'expediteur
//   DH2 = DH(EK_expediteur, IK_destinataire)    authentifie le destinataire
//   DH3 = DH(EK_expediteur, SPK_destinataire)   apporte le caractere ephemere
//
// racine = HKDF(DH1 || DH2 || DH3)
//
// Retirer DH1 rendrait l'expediteur usurpable ; retirer DH2 permettrait a
// quiconque detient une prekey volee de se faire passer pour le destinataire ;
// retirer DH3 lierait la session a des cles a longue duree de vie. Les trois
// sont necessaires, aucun n'est decoratif.
//
// Les deux fonctions calculent exactement la meme valeur : seuls les roles des
// cles s'echangent. C'est la propriete qui rend le protocole utilisable sans
// aucun aller-retour prealable -- l'expediteur peut ecrire a quelqu'un qui est
// hors ligne.

[[nodiscard]] bool derive_sender_session_key(
    identity_keypair const &sender_identity,
    x25519_secret_key const &sender_ephemeral_secret,
    ed25519_public_key const &recipient_identity,
    x25519_public_key const &recipient_prekey, symmetric_key &out);

[[nodiscard]] bool derive_recipient_session_key(
    identity_keypair const &recipient_identity,
    x25519_secret_key const &recipient_prekey_secret,
    ed25519_public_key const &sender_identity,
    x25519_public_key const &sender_ephemeral, symmetric_key &out);

} // namespace hypercom::crypto
