# SCHEMA — database

SQLite in WAL mode, a single file. The schema evolves **by adding
migrations** in `db/migrations/NNNN_description.sql`, never by editing
one retroactively.

## Non-negotiable rules

1. **Prepared statements only.** No string concatenation in SQL, no
   exceptions. There isn't even a constructor for assembling a query:
   `sql_statement` only takes a literal, values arrive through
   `sql_binder`.
2. **No `sqlite3_*` outside `server/db/`.** Business handlers all go
   through a repository.
3. **One repository per entity** (`user_repository`, `forum_repository`,
   …), no catch-all `database` class. That's a direct consequence of
   rule O3.
4. **Constraints live in the database**, not just in C++: an
   application bug shouldn't be able to produce an inconsistent row.

## Tables

| Table | Role | Points to watch |
|---|---|---|
| `users` | pubkey (32 bytes, `UNIQUE`), handle, dates | No email, no password. There's nothing to leak. |
| `prekeys` | one X25519 prekey signed per user | The server serves it, it can't forge it. |
| `forums` | unique name, founder, description, theme | No list of forbidden names. |
| `posts` | forum, author, title, body | Text only in v1. |
| `comments` | post, parent (`NULL` = root), author, body | Tree via `WITH RECURSIVE`, depth computed by the query. |
| `profiles` | display name, bio, theme, banner reference | `theme_json` is never interpreted by the server. |
| `friends` | unidirectional relation, status | Blocking is recorded, **never enforced** by the server. |
| `top8` | 8 ordered slots | `PRIMARY KEY (user_id, slot)`. |
| `dm_envelopes` | recipient, sender pubkey, ciphertext | The server CANNOT open it. |
| `blobs` | registry by hash | Registry only in v1, no bytes hosted. |
| `motd` | announcements | Partial unique index: only one active. |
| `schema_migrations` | applied migrations | Managed by `migration_runner`. |

## Details that aren't just details

**`dm_envelopes.sender_pubkey` is a raw key, not a foreign key.** A
deleted sender doesn't erase messages already received.

**Acknowledging a DM means deleting it.** What no longer exists on disk
can't be seized. `dm_repository::delete_acknowledged` filters on
`recipient_id`: nobody can wipe someone else's inbox by guessing IDs.

**`comments.depth` isn't stored.** It's computed by the recursive
query, so that moving a subtree can never desynchronize it. Depth is
capped **inside the query** (`MAX_COMMENT_DEPTH`), not after the fact.

**A forum's post count is a subquery**, not a denormalized column: a
hand-maintained counter always ends up drifting.

**Cryptographic sizes are enforced with `CHECK`.** A 31-byte key in the
database means a corrupted database, and that should be caught at
write time.

## PRAGMAs applied on open

```sql
PRAGMA journal_mode=WAL;      -- concurrent reads without blocking the writer
PRAGMA foreign_keys=ON;       -- off by default in sqlite (!)
PRAGMA busy_timeout=5000;     -- otherwise immediate SQLITE_BUSY under contention
PRAGMA synchronous=NORMAL;    -- good trade-off under WAL
PRAGMA temp_store=MEMORY;
```

`foreign_keys=ON` is the most important one: without it, every foreign
key constraint in the schema would be purely decorative.

## Backup

The database is **one file**. Backing it up means copying it — hot,
with `sqlite3 hypercom.db ".backup backup.db"`, which handles WAL
correctly. A raw copy taken mid-write can produce an inconsistent
file.
