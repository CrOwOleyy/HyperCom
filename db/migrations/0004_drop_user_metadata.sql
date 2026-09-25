-- Removal of metadata that only ever existed to observe.
--
-- What disappears, and why:
--
--   users.created_at         account age. Allows correlating accounts
--                            created at the same time, useful for
--                            nothing else.
--   friends.created_at       date two people became linked. That's a
--                            timeline of social relationships.
--   dm_envelopes.created_at  when someone received a private message.
--                            The server can't read the content, but it
--                            kept a timestamped log of exchanges. The
--                            date now lives INSIDE the ciphertext: the
--                            client displays it, the server no longer
--                            sees it.
--   dm_envelopes.delivered   dead column, never read by the code.
--
-- WHAT STAYS, deliberately: posts.created_at, comments.created_at and
-- forums.created_at. A forum where nobody can tell if a thread is from
-- today or two years ago stops being usable, and this content is public
-- by nature -- its timestamp is already visible to everyone reading it.
-- Removing it wouldn't protect anyone, it would only degrade the tool.

DROP INDEX idx_dm_recipient_undelivered;
CREATE INDEX idx_dm_recipient ON dm_envelopes(recipient_id, id);

ALTER TABLE dm_envelopes DROP COLUMN delivered;
ALTER TABLE dm_envelopes DROP COLUMN created_at;
ALTER TABLE users DROP COLUMN created_at;
ALTER TABLE friends DROP COLUMN created_at;
