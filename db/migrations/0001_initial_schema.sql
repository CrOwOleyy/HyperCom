-- Hypercom -- schema initial.
--
-- Propriete du collaborateur : ce fichier evolue par ajout de nouvelles
-- migrations, jamais par modification retroactive. Le serveur applique ce qui
-- manque au demarrage, sans recompilation.
--
-- Deux principes tenus partout :
--   1. Les contraintes sont dans la base, pas seulement dans le C++. Un bug
--      applicatif ne doit pas pouvoir produire une ligne incoherente.
--   2. Les tailles cryptographiques sont verifiees par CHECK. Une cle de 31
--      octets en base est une base corrompue, et on veut le savoir a l'ecriture.

CREATE TABLE users (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    pubkey     BLOB    NOT NULL UNIQUE CHECK (length(pubkey) = 32),
    handle     TEXT    NOT NULL UNIQUE,
    created_at INTEGER NOT NULL,
    last_seen  INTEGER NOT NULL DEFAULT 0
);

-- Une seule prekey courante par utilisateur en v1 : la republier remplace
-- l'ancienne. Le serveur ne fait que la servir, il ne peut pas la forger --
-- la signature est verifiable par le destinataire contre la cle d'identite.
CREATE TABLE prekeys (
    user_id    INTEGER PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    prekey     BLOB    NOT NULL CHECK (length(prekey) = 32),
    signature  BLOB    NOT NULL CHECK (length(signature) = 64),
    created_at INTEGER NOT NULL
);

CREATE TABLE forums (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT    NOT NULL UNIQUE,
    founder_id  INTEGER NOT NULL REFERENCES users(id) ON DELETE RESTRICT,
    description TEXT    NOT NULL DEFAULT '',
    theme_json  TEXT    NOT NULL DEFAULT '',
    created_at  INTEGER NOT NULL
);

CREATE TABLE posts (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    forum_id   INTEGER NOT NULL REFERENCES forums(id) ON DELETE CASCADE,
    author_id  INTEGER NOT NULL REFERENCES users(id) ON DELETE RESTRICT,
    title      TEXT    NOT NULL,
    body       TEXT    NOT NULL,
    created_at INTEGER NOT NULL
);

-- parent_comment_id NULL = reponse directe au post. L'arborescence se lit avec
-- WITH RECURSIVE ; depth est calculee par la requete, jamais stockee, pour
-- qu'un deplacement de sous-arbre ne puisse pas la desynchroniser.
CREATE TABLE comments (
    id                INTEGER PRIMARY KEY AUTOINCREMENT,
    post_id           INTEGER NOT NULL REFERENCES posts(id) ON DELETE CASCADE,
    parent_comment_id INTEGER REFERENCES comments(id) ON DELETE CASCADE,
    author_id         INTEGER NOT NULL REFERENCES users(id) ON DELETE RESTRICT,
    body              TEXT    NOT NULL,
    created_at        INTEGER NOT NULL
);

CREATE TABLE profiles (
    user_id      INTEGER PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    display_name TEXT NOT NULL DEFAULT '',
    bio          TEXT NOT NULL DEFAULT '',
    theme_json   TEXT NOT NULL DEFAULT '',
    banner_ref   TEXT NOT NULL DEFAULT ''
);

-- status : 0 demande, 1 acceptee, 2 bloquee. Le blocage est memorise ici mais
-- n'est PAS applique par le serveur : le filtrage reel vit dans le client.
-- Le serveur n'arbitre rien.
CREATE TABLE friends (
    user_id    INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    friend_id  INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    status     INTEGER NOT NULL DEFAULT 0 CHECK (status BETWEEN 0 AND 2),
    created_at INTEGER NOT NULL,
    PRIMARY KEY (user_id, friend_id),
    CHECK (user_id <> friend_id)
);

CREATE TABLE top8 (
    user_id   INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    slot      INTEGER NOT NULL CHECK (slot BETWEEN 0 AND 7),
    friend_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    PRIMARY KEY (user_id, slot)
);

-- Le serveur ne peut PAS ouvrir ciphertext. Il n'a aucune cle permettant de le
-- faire, et il ne stocke rien d'autre du message : ni objet, ni longueur du
-- clair, ni type. sender_pubkey est une cle brute et non une cle etrangere,
-- pour qu'un expediteur supprime n'efface pas les messages deja recus.
CREATE TABLE dm_envelopes (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    recipient_id  INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    sender_pubkey BLOB    NOT NULL CHECK (length(sender_pubkey) = 32),
    ciphertext    BLOB    NOT NULL,
    created_at    INTEGER NOT NULL,
    delivered     INTEGER NOT NULL DEFAULT 0
);

-- Registre seul en v1 : aucun octet de media n'est heberge. Le transfert P2P
-- de la v2 se branchera dessus sans changer le format des posts.
CREATE TABLE blobs (
    hash      BLOB PRIMARY KEY CHECK (length(hash) = 32),
    size      INTEGER NOT NULL,
    mime_hint TEXT    NOT NULL DEFAULT ''
);

CREATE TABLE motd (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    body       TEXT    NOT NULL,
    active     INTEGER NOT NULL DEFAULT 0,
    created_at INTEGER NOT NULL
);

CREATE INDEX idx_posts_forum_created    ON posts(forum_id, created_at DESC);
CREATE INDEX idx_posts_author           ON posts(author_id);
CREATE INDEX idx_comments_post          ON comments(post_id, id);
CREATE INDEX idx_comments_parent        ON comments(parent_comment_id);
CREATE INDEX idx_friends_friend         ON friends(friend_id);
CREATE INDEX idx_dm_recipient_undelivered
    ON dm_envelopes(recipient_id, id) WHERE delivered = 0;
CREATE UNIQUE INDEX idx_motd_single_active ON motd(active) WHERE active = 1;
