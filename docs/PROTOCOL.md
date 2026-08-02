# PROTOCOL — protocole de fil Hypercom v1

Protocole binaire maison. Pas de HTTP, pas de JSON, pas de texte.

## 1. Couches

```
  TCP  ──►  [u32 len][message Noise]        transport, chiffré
                          │
                          ▼ déchiffrement
            [u32 body_size][u8 type][payload]   trame applicative
```

Le préfixe de longueur externe est en clair — il le faut pour découper le
flux — mais **tout ce qu'il encadre est chiffré**, y compris l'octet de type.
Un observateur du réseau ne voit que des tailles de messages.

## 2. Handshake — `Noise_NK_25519_ChaChaPoly_SHA256`

```
NK:
  <- s          clé statique du serveur, épinglée par le client
  ...
  -> e, es      message 1
  <- e, ee      message 2  →  Split()
```

- **Prologue** : `hypercom-v1`, mixé par les deux pairs. Lie la session à cette
  application et à cette version.
- Le nom de suite fait exactement 32 octets, donc HASHLEN : il sert directement
  d'état de hachage initial, sans passer par SHA-256.
- **Sens des clés** : l'initiateur émet avec la première sortie de `Split()`,
  le répondeur avec la seconde. S'y tromper produit un canal qui s'établit puis
  échoue au premier message — d'où le test bidirectionnel.

Un client qui épingle la mauvaise clé **échoue dès le premier message**, sans
rien avoir révélé. C'est le remplacement complet de la chaîne de certificats :
pas d'autorité à interroger, seulement une clé à comparer.

## 3. Cadrage applicatif

```
[u32 body_size][u8 type][payload]
```

`body_size` compte l'octet de type et le payload, pas le champ lui-même :
`1 ≤ body_size ≤ MAX_FRAME_SIZE - 4`, soit 1 MiB au total.

**Règles non négociables du parseur** (`common/protocol/`) :

- Toute lecture passe par `byte_reader`, qui ne sort jamais de son tampon.
- Toute taille annoncée est comparée à son plafond **avant** la moindre
  allocation. Une trame annonçant 4 GiB est rejetée sans réserver un octet.
- Un échec ne consomme rien et n'écrit pas la sortie.
- Un type inconnu est rejeté par `decode_frame_header`, jamais transmis à un
  handler.

Le parseur est sans état, sans I/O et sans lien avec libsodium — précisément
pour être fuzzable seul (`tools/fuzz/`).

## 4. Encodage des champs

| Type | Encodage |
|---|---|
| entiers | petit-boutiste, taille fixe |
| texte | `[u32 taille][octets UTF-8]`, validé |
| blob | `[u32 taille][octets]` |
| clé publique / signature | 32 / 64 octets bruts |
| liste | `[u16 nombre][éléments]`, nombre plafonné |
| horodatage | `u64`, secondes UNIX UTC |

Le même préfixe `u32` sert aux textes et aux blobs : deux octets de plus par
chaîne, contre un seul chemin de décodage à auditer.

**Validation du texte** — un texte accepté est resservi tel quel à d'autres
clients, la validation n'est donc pas cosmétique. Sont refusés : UTF-8 invalide
ou sur-long, demi-codets de substitution, hors-plan Unicode, contrôles C0 sauf
tabulation et saut de ligne, DEL, et le retour chariot.

## 5. Séquence d'ouverture de session

```
C → S   hello_request      { version, clé publique }
S → C   auth_challenge     { nonce 32 o, version, compte_existe }
C → S   auth_response      { signature Ed25519 }
S → C   auth_accepted      { user_id, pseudo, heure }      si le compte existe
        ou status_ok       { 0 }                            sinon
C → S   register_request   { pseudo }                       le cas échéant
S → C   auth_accepted
S → C   motd_push                                           si un MOTD est actif
```

Le client signe `"hypercom-auth-v1" || nonce || clé_publique`. La séparation de
domaine empêche qu'une signature produite ici vaille dans un autre contexte.

Aucun mot de passe n'est transmis, stocké, ni même existant côté serveur.

## 6. Familles de messages

| Plage | Famille |
|---|---|
| `0x0*` | session : hello, auth, ping, motd, status |
| `0x1*` | compte : register, prekey publish/fetch |
| `0x2*` | forums : create, list |
| `0x3*` | contenu : post create/list, thread, comment |
| `0x4*` | social : profile, friends, top8 |
| `0x5*` | privé : dm send/fetch/ack |
| `0x6*` | blobs — **réservés v2**, valeurs posées pour que l'ajout du P2P ne renumérote rien |

Les valeurs sont figées : elles font partie du format de fil.

## 7. Enveloppe de message privé

Opaque pour le serveur, définie par `common/crypto/dm_envelope` :

```
[u8 version][32 identité expéditeur][32 éphémère][u32 compteur][ciphertext+tag]
└──────────────── donnée associée, authentifiée ────────────────┘
```

L'en-tête circule en clair — le destinataire en a besoin pour dériver la clé —
mais il est intégralement authentifié. Modifier un seul octet, y compris le
compteur, fait échouer le déchiffrement : le serveur ne peut ni rejouer à un
autre compteur, ni maquiller l'expéditeur.

Le nonce est nul, et c'est sûr **uniquement** parce que la clé est unique par
message : le cliquet de `dm_message_chain` n'en produit jamais deux fois la
même.

## 8. Codes d'erreur

Volontairement grossiers. `unknown_user` et `invalid_signature` répondent tous
deux `authentication_failed` : un code trop précis renseignerait un attaquant
sur l'état interne du serveur.
