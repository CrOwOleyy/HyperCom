# Architecture

Ce document explique comment les pièces s'assemblent : ce qui se passe
entre le moment où un client se connecte et celui où un message atterrit
dans un fil de discussion ou une boîte de DM. Pour le détail d'un sujet
précis, les autres docs vont plus loin :
[PROTOCOL.md](docs/PROTOCOL.md) pour le format des trames,
[SCHEMA.md](docs/SCHEMA.md) pour la base, [THREAT_MODEL.md](docs/THREAT_MODEL.md)
pour ce qui est protégé ou non, [ADMIN.md](docs/ADMIN.md) pour faire tourner
un serveur.

## Le serveur ne fait rien en parallèle

`server_runtime::run_until_stopped` est une seule boucle `epoll`. Pas de
thread par connexion, pas de pool. Chaque connexion, chaque requête à la
base, chaque calcul crypto passe l'un après l'autre dans la même boucle,
et aucun mutex n'existe nulle part dans `server/` — il n'y a rien à
protéger puisque rien ne tourne en parallèle. Deux requêtes qui toucheraient
la même ligne en même temps, un compteur de rate-limit corrompu par une
écriture concurrente : toute cette famille de bugs n'a simplement pas
d'endroit où se produire. Le compromis se paie en débit maximal, mais pour
un serveur qui sert une communauté plutôt qu'un service à millions
d'utilisateurs, la boucle unique ne devient jamais le goulot.

## Du socket au fil de discussion

Un message qui arrive traverse ces couches, dans cet ordre :

```
socket TCP
  → tampon d'octets bruts (connection_socket)
  → dé-préfixage longueur (extract_length_prefixed_message)
  → handshake Noise en cours, ou déchiffrement si déjà établi
  → décodage d'en-tête de trame (frame_codec)
  → limitation de débit (par adresse, puis par identité)
  → routage par famille de message (request_router)
  → handler (account_handler, forum_handler, dm_handler, ...)
  → repository (post_repository, dm_repository, ...)
  → SQLite
```

Les deux premières branches — handshake ou trame applicative — se
décident dans `connection_processor.cpp`. Tant que
`channel.is_established()` renvoie faux, chaque message reçu fait avancer
le handshake Noise au lieu d'être traité comme une requête ; une fois le
canal établi, tout ce qui arrive est déchiffré puis interprété comme une
trame.

Le routage est coupé en familles (session, contenu, social, DM,
signalement) plutôt qu'un unique `switch` sur tous les types de message :
la norme du projet plafonne une fonction à soixante lignes, et un switch
qui couvre les vingt et quelques types de message existants la
dépasserait largement. `route_message` essaie chaque famille dans l'ordre
et s'arrête dès que l'une d'elles reconnaît le type.

Chaque handler ne connaît que sa propre tâche — `handle_dm_send_request`
ignore tout de la table `posts`. Ce qu'il partage avec les autres, c'est le
`handler_context` (config, logger, connexion base, connexion cliente) et
les repositories, qui sont le seul endroit du code à toucher SQLite
directement. Un handler qui construirait sa propre requête SQL serait un
signal d'alarme à ce stade.

## Le canal chiffré

Le transport n'est pas TLS — il n'y a pas de client web à satisfaire, et
TLS traîne X.509 et les autorités de certification, deux choses dont ce
projet n'a aucun usage. À la place : Noise NK sur libsodium. Le client
connaît d'avance la clé publique statique du serveur (épinglée à la
première connexion, ou lue dans un fichier de connexion partagé) ; le
handshake échoue tout simplement si un serveur substitué tente de répondre
à sa place.

Une fois le handshake terminé (l'étape que le protocole Noise appelle
`Split`), chaque sens de communication a sa propre clé et son propre
compteur de nonce dans `noise_transport`. Un message du client et un
message du serveur ne peuvent donc jamais partager le même nonce — s'ils
le faisaient, ChaCha20-Poly1305 cesserait d'être sûr. Tout ce qui suit,
protocole applicatif compris, n'existe en clair que sur les deux machines
aux extrémités.

## Un client, plusieurs serveurs

Le client s'inspire de Discord plus que de Slack : une seule fenêtre, une
barre de serveurs sur le côté, et chaque serveur garde sa propre identité.
Une identité partagée entre deux serveurs serait un identifiant que deux
administrateurs pourraient recouper pour établir que c'est la même
personne des deux côtés — ce que le projet évite en donnant à chaque
serveur sa propre paire de clés, sans lien visible entre elles.

Deux structures se répartissent l'état :

- `app_state` contient ce qui appartient à l'application entière : la
  langue, le slot actif, la passphrase tenue en mémoire pour la session
  (jamais écrite sur disque).
- `server_slot` contient tout ce qui appartient à *un* serveur : sa
  connexion, son identité, sa session, et l'état de ce qui s'affiche pour
  lui.

Les slots vivent dans un `std::vector<std::unique_ptr<server_slot>>`,
jamais en valeur directe. La raison tient à un détail d'implémentation
facile à casser par accident : `client_session` garde des références vers
`server_connection` et vers l'identité du slot. Si le vecteur contenait
des `server_slot` par valeur, un `push_back` qui déclenche une
réallocation invaliderait ces références sans prévenir personne — le
`unique_ptr` fixe l'adresse du slot une fois pour toutes, donc ajouter un
serveur ne bouge jamais ceux qui existent déjà.

## Où regarder pour quoi

| Question | Dossier |
|---|---|
| Comment un message est structuré sur le fil | `common/protocol/` |
| Chiffrement, dérivation de clés, DM | `common/crypto/` |
| Boucle réseau, rate limiting, sessions | `server/net/` |
| Accès à SQLite | `server/db/` |
| Logique métier par type de message | `server/handlers/` |
| Commandes d'administration (socket local) | `server/admin/` |
| Connexion, keystore, multi-serveur | `client/net/`, `client/keystore/` |
| Interface ImGui | `client/ui/` |
