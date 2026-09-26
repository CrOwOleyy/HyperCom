# Contributing to HyperCom

Thanks for taking the time to look at this project. It's a prototype, built
and maintained by a small team, so keep expectations calibrated — but bug
reports and pull requests are welcome.

## Reporting a bug

Open an issue. Include:

- What you did, what you expected, what happened instead
- Your platform (Linux/WSL for the server, Windows or Linux for the
  client) and compiler
- Server log output if relevant (`log_peer_addresses` stays `false`, so
  logs are already safe to paste)

For a security vulnerability, do **not** open a public issue — see
[SECURITY.md](SECURITY.md) instead.

## Sending a pull request

- Keep the change focused. One subject per PR is easier to review than a
  bundle of unrelated fixes.
- Match the existing style: `snake_case`, a function stays under 60 lines,
  a source file exposes at most 5 public functions. `.clang-format` covers
  the mechanical part — run it before committing.
- Comments in code stay in English. `hypercom_cli`'s user-facing strings
  are the one deliberate exception and stay in French.
- Build with `-DHYPERCOM_PEDANTIC=ON` at least once before opening the PR
  — it's the warning set CI runs with, and catching issues locally is
  faster than waiting on a CI round trip.
- Run the test suite (`ctest --test-dir build --output-on-failure`). Add a
  test for new behavior when it's practical to isolate.

## What to expect from review

This touches cryptography and network-facing parsing code, so review is
deliberate, especially for anything under `common/crypto/`,
`common/protocol/`, or the Noise handshake path. Don't take a slow review
personally — a mistake there isn't the kind you want to find after
merging.

## Questions

Anything that doesn't fit an issue or a PR: reach out at
[leyy@pepepak.fr](mailto:leyy@pepepak.fr).
