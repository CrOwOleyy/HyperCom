-- Removal of the presence tracker.
--
-- users.last_seen was written on EVERY authentication and served to ANY
-- user by profile_get. Concretely, anyone could query someone's profile
-- every thirty seconds and reconstruct their connection times, habits,
-- and absences.
--
-- This is exactly what the project exists to make impossible, and it
-- wasn't even restricted to an administrator: it was a feature offered
-- to everyone.
--
-- The column disappears rather than being hidden from display: what
-- isn't written can't leak, nor be seized with the disk.
--
-- Consequence: no way left to know whether someone is online, or when
-- they last were. That's the intended behavior, not a side effect.

ALTER TABLE users DROP COLUMN last_seen;
