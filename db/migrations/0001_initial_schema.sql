-- Hypercom -- initial schema.
--
-- Owned by the collaborator: this file evolves by adding new migrations,
-- never by editing one retroactively. The server applies whatever is
-- missing on startup, without recompiling.
--
-- Two principles held throughout:
--   1. Constraints live in the database, not just in C++. An application
--      bug shouldn't be able to produce an inconsistent row.
--   2. Cryptographic sizes are enforced with CHECK. A 31-byte key in the
--      database means a corrupted database, and that should be caught at
--      write time.

CREATE TABLE users (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    pubkey     BLOB    NOT NULL UNIQUE CHECK (length(pubkey) = 32),
    handle     TEXT    NOT NULL UNIQUE,
    created_at INTEGER NOT NULL,
    last_seen  INTEGER NOT NULL DEFAULT 0
);

-- A single current prekey per user in v1: republishing replaces the
-- previous one. The server only serves it, it can't forge it -- the
-- signature is verifiable by the recipient against the identity key.
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

-- parent_comment_id NULL = direct reply to the post. The tree is read with
-- WITH RECURSIVE; depth is computed by the query, never stored, so that
-- moving a subtree can never desynchronize it.
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

-- status: 0 requested, 1 accepted, 2 blocked. Blocking is recorded here but
-- is NOT enforced by the server: real filtering lives in the client. The
-- server arbitrates nothing.
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

-- The server CANNOT open ciphertext. It has no key that would let it, and
-- it stores nothing else about the message: no object, no plaintext
-- length, no type. sender_pubkey is a raw key rather than a foreign key,
-- so that a deleted sender doesn't erase messages already received.
CREATE TABLE dm_envelopes (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    recipient_id  INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    sender_pubkey BLOB    NOT NULL CHECK (length(sender_pubkey) = 32),
    ciphertext    BLOB    NOT NULL,
    created_at    INTEGER NOT NULL,
    delivered     INTEGER NOT NULL DEFAULT 0
);

-- Registry only in v1: no media byte is hosted. The v2 P2P transfer will
-- plug into this without changing the post format.
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
