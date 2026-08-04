
## Administrer le serveur

Le serveur ouvre un socket local (`run/hypercom-admin.sock` par défaut) sur
lequel `hypercom_adminctl` envoie des commandes. Pas de C++ à écrire, pas de
SQL à taper.

hypercom_adminctl help
hypercom_adminctl stats
hypercom_adminctl sessions
hypercom_adminctl sessions close 12
hypercom_adminctl motd set "maintenance samedi 14h"
hypercom_adminctl motd clear
hypercom_adminctl backup sauvegardes/hypercom.db

Si le socket n'est pas au chemin par défaut :

hypercom_adminctl --socket /var/run/hypercom-admin.sock stats

Détails et garanties de sécurité : docs/ADMIN.md §7.

## Lancer les tests

Après chaque modification, recompiler puis lancer la suite :

cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure

Huit suites :

| Suite | Ce qu'elle couvre |
|---|---|
| `protocol_parsing_test` | bornes du lecteur, plafonds, validation UTF-8, cadrage |
| `crypto_round_trip_test` | handshake, DM chiffrés, keystore |
| `noise_official_vectors_test` | comparaison octet par octet à un vecteur officiel Noise |
| `message_roundtrip_session_test` | messages de session : hello, auth, ping, MOTD, status |
| `message_roundtrip_content_test` | forums, posts, commentaires, fils |
| `message_roundtrip_social_test` | compte, prekeys, profils, amis, top 8, DM |
| `fuzz_corpus_replay_test` | rejeu du corpus de fuzzing, sans libFuzzer |
| `reconnection_test` | re-handshake complet sur un même objet (POSIX seulement) |

Sous sanitizers :

cmake -S . -B build-asan -DHYPERCOM_SANITIZER=address,undefined
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure

TSAN se lance dans un répertoire séparé (incompatible avec ASAN). Sous WSL, il
faut désactiver l'ASLR, sinon il refuse de démarrer :

setarch -R ctest --test-dir build-tsan --output-on-failure

Un test seul, pour voir le détail :

build\windows\bin\RelWithDebInfo\noise_official_vectors_test.exe

## Sur Windows (PowerShell)

### 1. Installer les dépendances
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\scripts\fetch_third_party.ps1

### 2. Compiler (Générer l'exécutable)

cmake -S . -B build/windows

### 3. Compiler le projet
cmake --build build/windows --config RelWithDebInfo -j

### 3. Lancer le projet
### 1. Charger l'environnement
. .\env.ps1

### Pour lancer le serveur localement dans le terminal 1
hserver

### 2. Lancer le client interface graphique (GUI)
hgui

### Pour lancer un 2ème client (pour tester avec un 2ème compte)
hgui --identity compte2.key


## La séquence d'accueil

Elle ne se joue **qu'à la création d'un compte**, jamais aux connexions
suivantes. Pour la revoir sans créer de compte jetable :

hgui --replay-intro

Ou en repartant d'une identité neuve — attention, le fichier ne doit pas déjà
exister, sinon le compte est déjà enregistré et l'intro ne se déclenche pas :

hgui --identity compte_neuf.key

Déroulé, calé sur la durée réelle de `menu.mp3` (~12,5 s) :

| Moment | Ce qui se passe |
|---|---|
| 0 → 6 s | Carte de verre centrée, « Bienvenue dans l'espace HyperCom. » |
| 6 s → fin | Les trois colonnes remontent l'une après l'autre, en bulles |
| ensuite | Interface normale, plus aucune animation |

Remplacer `menu.mp3` recale l'animation tout seul : la durée est lue dans le
fichier. CMake le recopie à côté de l'exécutable à chaque build.

Pas de son ? Ce n'est jamais bloquant — l'intro se déroule à l'identique sur
l'horloge. Le client affiche la raison au démarrage :

    intro : musique, duree retenue 12.5268 s



## Sur Linux / WSL

### 1. Installer les dépendances
./scripts/fetch_third_party.sh

### 2. Compiler (Générer l'exécutable)
# Générer les fichiers de construction
cmake -S . -B build/linux

# Compiler le projet
cmake --build build/linux -j

### 3. Lancer le projet
*(Charge les raccourcis `hgui`, `hcli` et `hserver`)*

### 1. Pour lancer le serveur localement (terminal 1)
hserver

### 2. Charger l'environnement (terminal 2)
source env.sh

### 3. Lancer le client interface graphique (GUI) (terminal 2)
hgui


