-- Reporting and banning.
--
-- Two legal obligations distinct from the encrypted content: a reporting
-- mechanism (LCEN art. 6-I-7) and the ability to act once informed.
-- Neither one grants access to a DM's content -- a DM is reported by
-- account, never by a post_id that doesn't exist for it.

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
