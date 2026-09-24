# HyperCom

[English](../../README.md) · **Français** · [中文](README.zh.md) · [हिन्दी](README.hi.md) · [Español](README.es.md) · [العربية](README.ar.md) · [বাংলা](README.bn.md) · [Português](README.pt.md) · [Русский](README.ru.md) · [日本語](README.ja.md)

Réseau social décentralisé et chiffré de bout en bout : forums communautaires
façon Reddit, profils façon MySpace, messages privés qu'aucun serveur ne peut
lire. Écrit from scratch en C++20, sans TLS ni dépendance web — protocole
Noise fait maison sur libsodium.

Chaque communauté héberge son propre serveur, comme sur Discord, plutôt qu'un
service central. Une seule identité maîtresse permet de rejoindre autant de
serveurs qu'on veut, chacun avec une identité distincte et non corrélable.

Documentation complète : [ARCHITECTURE.md](../../ARCHITECTURE.md) (comment
les pièces s'assemblent), [BRIEF.md](../../BRIEF.md) (décisions de
conception), [docs/ADMIN.md](../ADMIN.md) (exploitation serveur),
[docs/THREAT_MODEL.md](../THREAT_MODEL.md) (ce qui est protégé, ce qui ne
l'est pas), [COMMANDS.md](../../COMMANDS.md) (référence complète des
commandes).

## Prérequis

- CMake ≥ 3.20, compilateur C++20 (MSVC sur Windows, GCC/Clang sur Linux)
- libsodium, SQLite et Dear ImGui : récupérés une fois via un script, jamais
  automatiquement par CMake (BRIEF.md 15)

**Le serveur ne compile que sous Linux/WSL** (il s'appuie sur epoll et
signalfd). Le client — CLI et interface graphique — compile sous Windows et
sous Linux.

## Côté serveur (Linux / WSL)

```bash
./scripts/fetch_third_party.sh
cmake -S . -B build/linux
cmake --build build/linux -j
```

Première configuration :

```bash
./build/linux/bin/hypercom_keygen server keys/server_static.key
chmod 600 keys/server_static.key
```

Créer `hypercom.conf` à la racine (toutes les options sont commentées dans
`docs/ADMIN.md §1-2`) :

```ini
[server]
registration_open = true

[clearnet]
enabled      = true
bind_address = 0.0.0.0
port         = 7717

[onion]
enabled = false

[limits]
max_connections           = 512
handshake_timeout_seconds = 10
idle_timeout_seconds      = 0

[paths]
database     = hypercom.db
server_key   = keys/server_static.key
admin_socket = run/hypercom-admin.sock
```

Puis :

```bash
./build/linux/bin/hypercom_server hypercom.conf
```

Au démarrage, le serveur affiche sa clé publique et écrit
`run/hypercom-connect.txt`. C'est ce fichier — ou la clé qu'il contient —
qu'on transmet à quelqu'un pour qu'il rejoigne le serveur, par un canal de
confiance et jamais en le faisant récupérer depuis le serveur lui-même.

Administrer un serveur en cours d'exécution (sessions, MOTD, signalements,
bannissement) : `docs/ADMIN.md §7`.

## Côté client (Windows ou Linux)

```powershell
.\scripts\fetch_third_party.ps1
cmake -S . -B build/windows
cmake --build build/windows --config RelWithDebInfo -j
```

```bash
./scripts/fetch_third_party.sh
cmake -S . -B build/linux
cmake --build build/linux -j
```

Charger l'environnement une fois construit — ça expose les raccourcis
`hserver`, `hgui` et `hcli` :

```powershell
cp env.example.ps1 env.ps1   # une seule fois, puis éditer avec vos valeurs
. .\env.ps1
```

Rejoindre un serveur avec le lien d'invitation reçu
(`hypercom://hote:port#cle`) :

```
hcli server-add hypercom://203.0.113.7:7717#447a6def... mon-serveur
hcli --server mon-serveur whoami
```

Ou directement avec le fichier de connexion transmis par l'administrateur :

```
hcli --connect-file hypercom-connect.txt whoami
```

Lancer l'interface graphique :

```
hgui
```

Un deuxième compte, pour tester en local avec deux identités côte à côte :

```
hgui --identity compte2.key
```

## Tester

```bash
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

Détail des suites, builds sous sanitizers et séquence d'accueil du client :
voir [COMMANDS.md](../../COMMANDS.md).
