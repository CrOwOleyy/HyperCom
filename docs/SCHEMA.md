# SCHEMA — base de données

SQLite en mode WAL, un seul fichier. Le schéma évolue **par ajout de migrations**
dans `db/migrations/NNNN_description.sql`, jamais par modification rétroactive.

## Règles non négociables

1. **Requêtes préparées exclusivement.** Aucune concaténation de chaîne dans du
   SQL, sans aucune exception. Il n'existe même pas de constructeur permettant
   d'assembler une requête : `sql_statement` ne prend qu'un littéral, les
   valeurs arrivent par `sql_binder`.
2. **Aucun `sqlite3_*` en dehors de `server/db/`.** Les handlers métier passent
   tous par un dépôt.
3. **Un dépôt par entité** (`user_repository`, `forum_repository`, …), pas de
   classe `database` fourre-tout. C'est la conséquence directe de la règle O3.
4. **Les contraintes sont dans la base**, pas seulement dans le C++ : un bug
   applicatif ne doit pas pouvoir produire une ligne incohérente.

## Tables

| Table | Rôle | Points d'attention |
|---|---|---|
| `users` | pubkey (32 o, `UNIQUE`), pseudo, dates | Aucun email, aucun mot de passe. Il n'y a rien à fuiter. |
| `prekeys` | une prekey X25519 signée par utilisateur | Le serveur la sert, il ne peut pas la forger. |
| `forums` | nom unique, fondateur, description, thème | Aucune liste de noms interdits. |
| `posts` | forum, auteur, titre, corps | Texte seul en v1. |
| `comments` | post, parent (`NULL` = racine), auteur, corps | Arborescence via `WITH RECURSIVE`, profondeur calculée par la requête. |
| `profiles` | nom affiché, bio, thème, référence de bannière | `theme_json` n'est jamais interprété par le serveur. |
| `friends` | relation unidirectionnelle, statut | Le blocage est mémorisé, **jamais appliqué** par le serveur. |
| `top8` | 8 emplacements ordonnés | `PRIMARY KEY (user_id, slot)`. |
| `dm_envelopes` | destinataire, pubkey expéditeur, ciphertext | Le serveur ne peut PAS l'ouvrir. |
| `blobs` | registre par hash | Registre seul en v1, aucun octet hébergé. |
| `motd` | annonces | Index unique partiel : un seul actif. |
| `schema_migrations` | migrations appliquées | Géré par `migration_runner`. |

## Détails qui ne sont pas des détails

**`dm_envelopes.sender_pubkey` est une clé brute, pas une clé étrangère.**
Un expéditeur supprimé n'efface pas les messages déjà reçus.

**Acquitter un DM, c'est le supprimer.** Ce qui n'existe plus sur le disque ne
peut pas être saisi. `dm_repository::delete_acknowledged` filtre sur
`recipient_id` : personne ne peut effacer la boîte d'un autre en devinant des
identifiants.

**`comments.depth` n'est pas stockée.** Elle est calculée par la requête
récursive, pour qu'un déplacement de sous-arbre ne puisse pas la
désynchroniser. La profondeur est plafonnée **dans la requête**
(`MAX_COMMENT_DEPTH`), pas après coup.

**Le nombre de posts d'un forum est une sous-requête**, pas une colonne
dénormalisée : un compteur maintenu à la main finit toujours par diverger.

**Les tailles cryptographiques sont vérifiées par `CHECK`.** Une clé de
31 octets en base est une base corrompue, et on veut le savoir à l'écriture.

## PRAGMA appliqués à l'ouverture

```sql
PRAGMA journal_mode=WAL;      -- lectures concurrentes sans bloquer l'écrivain
PRAGMA foreign_keys=ON;       -- désactivé par défaut par sqlite (!)
PRAGMA busy_timeout=5000;     -- sinon SQLITE_BUSY immédiat sous contention
PRAGMA synchronous=NORMAL;    -- bon compromis en WAL
PRAGMA temp_store=MEMORY;
```

`foreign_keys=ON` est le plus important : sans lui, toutes les contraintes de
clé étrangère du schéma seraient purement décoratives.

## Sauvegarde

La base est **un fichier**. Sauvegarder, c'est le copier — à chaud avec
`sqlite3 hypercom.db ".backup sauvegarde.db"`, qui gère le WAL correctement.
Une copie brute pendant une écriture peut produire un fichier incohérent.
