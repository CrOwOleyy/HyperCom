-- Signalement et bannissement.
--
-- Deux obligations legales distinctes du contenu chiffre : un dispositif de
-- signalement (LCEN art. 6-I-7) et la capacite d'agir une fois informe. Aucune
-- des deux ne donne acces au contenu d'un DM -- un DM se signale par le
-- compte, jamais par un post_id qui n'existe pas pour lui.

ALTER TABLE users ADD COLUMN banned INTEGER NOT NULL DEFAULT 0
    CHECK (banned IN (0, 1));

CREATE TABLE reports (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    kind          TEXT    NOT NULL CHECK (kind IN ('post', 'account')),
    post_id       INTEGER REFERENCES posts(id) ON DELETE CASCADE,
    target_pubkey BLOB    CHECK (target_pubkey IS NULL OR length(target_pubkey) = 32),
    reporter_id   INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    reason        TEXT    NOT NULL DEFAULT '',
    created_at    INTEGER NOT NULL
);

CREATE INDEX idx_reports_created ON reports(created_at);
