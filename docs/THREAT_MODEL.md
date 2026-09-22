# THREAT_MODEL — ce que Hypercom protège, et ce qu'il ne protège pas

Ce document est écrit pour être montré aux utilisateurs. Il ne promet rien que
l'architecture ne tienne.

## 1. Le principe

**La non-surveillance est une propriété technique, pas une promesse.**
L'opérateur du serveur ne doit pas *choisir* de ne pas lire les conversations
privées — il doit en être *incapable*.

Concrètement : il n'existe nulle part sur la machine serveur de clé permettant
d'ouvrir un message privé. Ce n'est pas une politique qu'un administrateur
pourrait changer, c'est une absence.

## 2. Ce qui est protégé

| Menace | Protection |
|---|---|
| Écoute du réseau | Tout est chiffré par Noise_NK, y compris le type de trame. |
| Serveur substitué | Clé statique épinglée : le handshake échoue avant tout échange. |
| Opérateur du serveur lisant les DM | Il n'a aucune clé. Le chiffrement se fait sur le client. |
| Serveur forgeant une prekey pour s'interposer | La prekey est signée par la clé d'identité ; le destinataire vérifie. |
| Serveur rejouant ou modifiant un DM | L'en-tête entier est donnée associée authentifiée. |
| Vol de la base de données | Elle ne contient que du chiffré opaque et des posts publics. |
| Saisie du serveur | Aucune IP journalisée par défaut, aucun email, aucun mot de passe. |
| Usurpation de compte | L'identité *est* une clé privée qui ne quitte jamais la machine. |
| Compromission future de la clé de chaîne | Cliquet symétrique : les messages passés restent illisibles. |
| Injection SQL | Requêtes préparées exclusivement, aucune concaténation. |
| Trame hostile | Parseur borné, plafonds vérifiés avant allocation, fuzzé. |
| Admin traçant la présence des utilisateurs | La CLI d'admin ne peut pas lister qui est en ligne : `sessions` ne montre ni pseudo ni adresse, et ce n'est pas réglable. |
| **N'importe qui** traçant la présence d'un autre | Aucune date de dernière connexion n'est stockée ni servie. La colonne `last_seen` a été supprimée du schéma et du protocole. |
| Serveur datant les échanges privés | L'horodatage vit **dans** le chiffré. Le serveur ne sait plus quand un message a été envoyé, seulement dans quel ordre les enveloppes sont arrivées. |
| Corrélation par âge de compte ou de relation | `users.created_at` et `friends.created_at` supprimées : ni date d'inscription, ni chronologie des liens sociaux. |
| Usurpation par la casse du pseudo | Unicité `COLLATE NOCASE` : `alice`, `Alice` et `ALICE` sont le même pseudo. |
| Contournement de la limite de débit par reconnexion | Les compteurs sont partagés entre connexions, pas remis à zéro à chacune. |
| Serveur recousant deux connexions d'une même personne | Aucun jeton de reprise n'existe. Une reconnexion est un handshake Noise neuf, avec une clé éphémère neuve. Voir PROTOCOL.md §9. |
| Sessions mortes s'accumulant après une coupure | Keepalive TCP réglé des deux côtés (240 s de détection), sans délai d'inactivité applicatif qui obligerait à mesurer l'activité de chacun. |

## 3. Ce qui n'est PAS protégé — limites assumées

### 3.1 Les métadonnées de routage

**Le serveur voit qui écrit à qui.** Il stocke `recipient_id` et
`sender_pubkey` — sans eux, il ne saurait pas à qui remettre l'enveloppe.

Il ne sait en revanche **plus quand** : l'horodatage est passé à l'intérieur du
chiffré, et l'ordre d'arrivée (`id`) est tout ce qui reste. Un serveur saisi
révèle donc le graphe des échanges, pas leur chronologie.

Masquer le graphe lui-même demanderait un mix-net ou des boîtes aveugles, hors
périmètre v1. C'est la limite la plus importante de ce modèle, et elle doit
être dite franchement aux utilisateurs.

### 3.1 bis Ce que le serveur détient réellement d'une personne

Après un usage complet (inscription, forum, post, commentaire, ami, message
privé), voici l'intégralité de ce que contient la base :

```
users        id, pubkey, handle, banned
friends      user_id, friend_id, status
top8         user_id, slot, friend_id
profiles     user_id, display_name, bio, theme_json, banner_ref
dm_envelopes id, recipient_id, sender_pubkey, ciphertext
reports      id, kind, post_id, target_pubkey, reporter_id, reason, created_at
```

Aucune date sur aucune de ces lignes, sauf `reports` : un signalement porte
nécessairement un horodatage, sinon l'admin ne peut pas savoir quand il traite
une file. Les autres horodatages qui subsistent portent sur du **contenu
public** — `posts`, `comments`, `forums` — où ils sont de toute façon visibles
de quiconque lit le fil, et sur `motd`, qui est une annonce d'administration.

`reports` est une exception délibérée à « pas de dates » : signaler quelque
chose révèle forcément qui a signalé quoi et quand (BRIEF.md 13). C'est un
choix assumé, pas un oubli — le dispositif de signalement est une obligation
légale, pas une fonctionnalité de surveillance déguisée, et son contenu
n'est lisible que par l'admin sur le socket local, jamais par le réseau.

**Hors base, dans le journal seulement, si `log_peer_addresses=true` :** une
ligne par connexion clearnet acceptée, adresse + horodatage. Jamais pour le
`.onion`, quel que soit ce réglage — Tor relaie en boucle locale, il n'y a
structurellement pas de vraie IP à voir côté serveur. Rien n'est écrit par
défaut.

### 3.2 Pas de confidentialité persistante future

Le cliquet est symétrique. Quelqu'un qui compromet la clé d'identité d'un
utilisateur peut lire ses messages **futurs**. Le cliquet Diffie-Hellman
complet, qui refermerait cette fenêtre, est prévu en v2 — le format
d'enveloppe est déjà prêt à l'accueillir.

### 3.3 Prekey non tournante

La prekey est dérivée déterministiquement de l'identité et ne tourne pas.
La confidentialité persistante repose donc entièrement sur la clé éphémère,
régénérée à chaque message. Acceptable, mais moins robuste qu'une rotation.

### 3.4 Clé perdue = compte perdu

Sans recours, définitivement. C'est le prix de l'absence d'autorité : personne
ne peut réinitialiser un compte, donc personne ne peut en voler un par ce
moyen. Une phrase de récupération à 12 mots est prévue en v2.

### 3.5 Implémentation maison de Noise

Le code de `common/crypto/noise_*` est une implémentation maison d'un protocole
**spécifié**, pas une cryptographie maison : toutes les primitives viennent de
libsodium. Le risque résiduel est une erreur dans l'enchaînement des étapes,
pas dans les primitives.

**Fait** : `tests/noise_official_vectors_test.cpp` rejoue le handshake avec les
clés fixes d'un vecteur officiel (source noise-c, `Noise_NK_25519_ChaChaPoly_
SHA256`, sans PSK) et compare chaque octet — messages de handshake, hachage
final, clés de transport, quatre messages de transport — à la référence.

Ce test couvre un angle mort que le round-trip ne couvre pas : un bug présent
à l'identique côté client et côté serveur (mauvais ordre de `mix_hash`, par
exemple) resterait invisible à deux parties qui se parlent entre elles mais se
trompent de la même façon. Comparer à une référence externe est le seul moyen
de l'attraper.

### 3.6 Le premier contact avec la clé du serveur

L'épinglage ne protège que si la clé arrive par un canal de confiance. Si un
utilisateur la récupère depuis le serveur lui-même, ou depuis un site que
l'attaquant contrôle, l'épinglage ne protège de rien.

### 3.7 Aucune modération — sauf l'exception légale

C'est le point du projet, mais c'est aussi une exposition. Le filtrage vit dans
le client de chacun.

Une exception unique existe : `reports`/`ban` sur le socket d'administration
local (BRIEF.md 13, 15). Elle ne change rien à ce qu'un client peut faire —
aucun message du protocole réseau ne permet d'effacer le contenu de
quelqu'un d'autre, seul son propre contenu reste supprimable. Ce que ça
change : un administrateur qui a la main sur la machine peut désormais bannir
un compte ou supprimer un post spécifiquement signalé, en réaction à une
obligation légale précise, jamais par jugement éditorial général.

### 3.8 Contenu des posts

Les posts sont **publics par nature**. Rien ne les chiffre, et rien ne devrait :
un forum lisible seulement par son auteur n'est pas un forum.

## 4. Hypothèses

- La machine du client n'est pas compromise. Si elle l'est, la clé privée l'est
  aussi, quelle que soit la qualité du protocole.
- libsodium est correct.
- L'utilisateur choisit une passphrase résistante. Argon2id (paramètres
  MODERATE, 256 MiB) ralentit une attaque hors ligne, il ne la rend pas
  impossible sur une passphrase faible.

## 5. État du durcissement

| Mesure | État |
|---|---|
| `-Wall -Wextra -Werror`, stack protector, RELRO, PIE | **fait**, appliqué à toutes les cibles |
| Cibles ASAN / UBSAN / TSAN | **fait**, `-DHYPERCOM_SANITIZER=...` |
| Suite de tests passant sous ASAN+UBSAN | **fait** |
| Harnais de fuzzing du parseur | **fait**, couvre les 36 décodeurs. Campagne continue à mettre en place : libFuzzer exige clang, absent de l'environnement actuel |
| Rejeu du corpus de fuzzing dans la suite | **fait**, `tests/fuzz_corpus_replay_test.cpp` rejoue les cas limites connus sous gcc et sous sanitizers, sans libFuzzer |
| Aller-retour de tous les messages | **fait**, trois suites `message_roundtrip_*` : chaque message encodé puis décodé, avec vérification qu'il ne reste aucun octet — seule façon de détecter un décalage de champ silencieux |
| Reconnexion sans jeton de reprise | **fait**, `tests/reconnection_test.cpp`, validé aussi sous TSAN |
| Requêtes préparées exclusivement | **fait** |
| Aucune globale mutable (G4) | **fait**, y compris l'arrêt par `signalfd` plutôt qu'un drapeau global |
| Pas de journalisation d'IP par défaut | **fait** |
| Vecteurs de test officiels Noise | **fait**, `tests/noise_official_vectors_test.cpp` |
| Purge automatique des journaux | **configurée, non appliquée** — `retention_days` est lu mais aucune rotation n'est implémentée |
| Abandon des privilèges après bind | **non fait** |
| seccomp-bpf + espaces de noms | **non fait** |
| Unité systemd durcie | **non fait** |

Les trois dernières lignes relèvent du déploiement et du domaine du
collaborateur.
