-- Deletion of one's own content by its author.
--
-- The network has no moderation: nobody can erase someone else's
-- content, neither a user nor an administrator. Everyone does stay in
-- control of what they themselves published, though -- that's the only
-- deletion the protocol allows, and it's checked server-side by a WHERE
-- author_id that makes impersonation impossible rather than merely
-- arbitrable.
--
-- Why a column and not a DELETE: the row has to survive for the comment
-- tree to hold together. Deleting a comment in the middle of a thread
-- would take every reply attached to it down with it (ON DELETE
-- CASCADE), meaning other people's content -- exactly what the project
-- forbids.
--
-- The text itself is genuinely erased on deletion (title and body set to
-- an empty string by the server), not just hidden from display. Hiding
-- it client-side would leave the content in the database and in every
-- backup, which wouldn't be a deletion.
--
-- deleted_at carries the date rather than a plain boolean: same storage
-- cost, and the information is useful to an administrator inspecting the
-- database. NULL means "visible".

ALTER TABLE posts ADD COLUMN deleted_at INTEGER;
ALTER TABLE comments ADD COLUMN deleted_at INTEGER;
