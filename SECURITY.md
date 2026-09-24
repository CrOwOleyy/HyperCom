# Security policy

**English** · [Français](docs/security/SECURITY.fr.md) · [中文](docs/security/SECURITY.zh.md) · [हिन्दी](docs/security/SECURITY.hi.md) · [Español](docs/security/SECURITY.es.md) · [العربية](docs/security/SECURITY.ar.md) · [বাংলা](docs/security/SECURITY.bn.md) · [Português](docs/security/SECURITY.pt.md) · [Русский](docs/security/SECURITY.ru.md) · [日本語](docs/security/SECURITY.ja.md)

Hypercom is maintained by a small team, outside of any professional
setting. This document says honestly what that implies.

## Reporting a vulnerability

**Do not open a public issue for a security vulnerability.** Use this
repository's [private vulnerability reporting](../../security/advisories/new)
(*Security* tab → *Report a vulnerability*): the report stays visible only
to maintainers until a fix ships.

Describe:

- the file and function involved, if possible;
- exact conditions to reproduce the problem;
- the concrete impact (what an attacker gains, not just "this looks
  wrong").

## What's in scope

Any flaw that makes one of the guarantees listed in
[docs/THREAT_MODEL.md](docs/THREAT_MODEL.md) false — for example: a DM
readable without the recipient's key, a bypass of signature-based
authentication, a SQL injection, a remotely triggerable crash without
authentication, a secret leaking through memory.

## What isn't a vulnerability

`THREAT_MODEL.md` documents **accepted** limitations, not oversights: the
server sees who's writing to whom, "friends only" isn't encryption, a
malicious server operator can lie to their own users. Reports on these
specific points will be closed with a pointer to that document rather
than treated as a bug.

## Supported versions

Hypercom doesn't have a numbered stable release yet: only the `main`
branch receives security fixes.

## Response time

No formal SLA — this is a two-person project, not a company. In
practice: acknowledgment within a week, a fix or an action plan
communicated before the report is made public.
