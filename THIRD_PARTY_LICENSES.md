# Dépendances tierces

HyperCom lui-même est sous licence MIT (voir `LICENSE`).

Aucune de ces dépendances n'est versionnée dans ce dépôt : elles sont
récupérées par `scripts/fetch_third_party.sh` (ou `.ps1` sous Windows), qui
vérifie l'empreinte SHA-256 de chaque archive contre `third_party/checksums.txt`
avant installation. Voir `third_party/README.md` pour le détail.

| Dépendance | Licence | Rôle |
|---|---|---|
| [libsodium](https://libsodium.org) | ISC | Toute la cryptographie : Ed25519, X25519, XChaCha20-Poly1305, Argon2id |
| [SQLite](https://sqlite.org) | Domaine public | Stockage, amalgamation compilée dans le binaire serveur |
| [Dear ImGui](https://github.com/ocornut/imgui) | MIT | Interface du client graphique natif |
| [GLFW](https://www.glfw.org) | zlib/libpng | Fenêtrage et contexte OpenGL du client graphique (Linux, et build GLFW sous Windows) |
| [miniaudio](https://github.com/mackron/miniaudio) | Domaine public (ou MIT-0, au choix) | Lecture du thème d'accueil, client graphique uniquement |

Le texte complet de chaque licence accompagne la distribution officielle de la
dépendance concernée, récupérée par le script ci-dessus.

## Périmètre

`libsodium` et `SQLite` sont liés au serveur, au client CLI et au client
graphique. `Dear ImGui`, `GLFW` et `miniaudio` ne sont liés qu'au client
graphique — ni le serveur ni le client CLI n'en dépendent, et leur absence
n'empêche pas de les construire.
