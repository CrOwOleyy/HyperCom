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

## 3. Ce qui n'est PAS protégé — limites assumées

### 3.1 Les métadonnées de routage

**Le serveur voit qui écrit à qui, et quand.** Il stocke `recipient_id`,
`sender_pubkey` et un horodatage.

Masquer cela demanderait un mix-net ou des boîtes aveugles, hors périmètre v1.
C'est la limite la plus importante de ce modèle, et elle doit être dite
franchement aux utilisateurs : le contenu est protégé, le graphe social ne
l'est pas.

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

**À faire, et non fait à ce jour** : valider contre les vecteurs de test
officiels du projet Noise. Les tests actuels vérifient le round-trip complet
dans les deux sens et le refus d'une clé mal épinglée, ce qui attrape les
erreurs grossières mais pas une divergence subtile avec la spécification.

### 3.6 Le premier contact avec la clé du serveur

L'épinglage ne protège que si la clé arrive par un canal de confiance. Si un
utilisateur la récupère depuis le serveur lui-même, ou depuis un site que
l'attaquant contrôle, l'épinglage ne protège de rien.

### 3.7 Aucune modération

C'est le point du projet, mais c'est aussi une exposition. Le filtrage vit dans
le client de chacun. Voir aussi BRIEF.md §13 sur les contraintes juridiques.

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
| Harnais de fuzzing du parseur | **fait**, campagne continue à mettre en place |
| Requêtes préparées exclusivement | **fait** |
| Aucune globale mutable (G4) | **fait**, y compris l'arrêt par `signalfd` plutôt qu'un drapeau global |
| Pas de journalisation d'IP par défaut | **fait** |
| Purge automatique des journaux | **configurée, non appliquée** — `retention_days` est lu mais aucune rotation n'est implémentée |
| Abandon des privilèges après bind | **non fait** |
| seccomp-bpf + espaces de noms | **non fait** |
| Unité systemd durcie | **non fait** |
| Vecteurs de test officiels Noise | **non fait** |

Les quatre dernières lignes relèvent du déploiement et du domaine du
collaborateur (BRIEF.md §10).
