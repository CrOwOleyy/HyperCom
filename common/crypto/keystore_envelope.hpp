#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

#include "common/crypto/key_types.hpp"

namespace hypercom::crypto {

// Format de la cle privee au repos, sur le disque du client :
//
//   "HYPCKEY1" | u8 version | u32 ops | u32 mem_kib | 16 sel | 24 nonce | scelle
//
// passphrase --Argon2id--> cle --XChaCha20-Poly1305--> cle privee scellee
//
// Les parametres Argon2id sont ecrits DANS le fichier plutot que codes en dur.
// Sans cela, augmenter le cout un jour rendrait illisibles toutes les cles
// existantes -- ce qui, sur ce projet, signifie perdre les comptes.
constexpr std::size_t KEYSTORE_MAGIC_SIZE = 8;
constexpr std::uint8_t KEYSTORE_VERSION = 1;

// Meme format, pour un contenu de taille quelconque. Sert a la graine maitresse
// (32 octets) et au registre des serveurs (taille variable) : la liste des
// serveurs qu'on frequente revele des appartenances, elle ne reste donc pas en
// clair sur le disque.
[[nodiscard]] bool seal_blob(std::string_view passphrase,
                             std::span<std::uint8_t const> plaintext,
                             std::vector<std::uint8_t> &out);

[[nodiscard]] bool open_blob(std::string_view passphrase,
                             std::span<std::uint8_t const> sealed,
                             std::vector<std::uint8_t> &out);

[[nodiscard]] bool seal_identity_secret(std::string_view passphrase,
                                        ed25519_secret_key const &secret,
                                        std::vector<std::uint8_t> &out);

// Renvoie false aussi bien pour une passphrase fausse que pour un fichier
// altere : le poly1305 ne distingue pas les deux, et c'est tres bien ainsi.
[[nodiscard]] bool open_identity_secret(std::string_view passphrase,
                                        std::span<std::uint8_t const> sealed,
                                        ed25519_secret_key &out);

} // namespace hypercom::crypto
