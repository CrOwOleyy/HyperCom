# Third-party dependencies

HyperCom itself is MIT licensed (see `LICENSE`).

None of these dependencies is vendored in this repository: they're
fetched by `scripts/fetch_third_party.sh` (or `.ps1` on Windows), which
verifies the SHA-256 checksum of each archive against
`third_party/checksums.txt` before installing. See `third_party/README.md`
for details.

| Dependency | License | Role |
|---|---|---|
| [libsodium](https://libsodium.org) | ISC | All cryptography: Ed25519, X25519, XChaCha20-Poly1305, Argon2id |
| [SQLite](https://sqlite.org) | Public domain | Storage, amalgamation compiled into the server binary |
| [Dear ImGui](https://github.com/ocornut/imgui) | MIT | Native graphical client interface |
| [GLFW](https://www.glfw.org) | zlib/libpng | Windowing and OpenGL context for the graphical client (Linux, and GLFW builds on Windows) |
| [miniaudio](https://github.com/mackron/miniaudio) | Public domain (or MIT-0, at your option) | Welcome theme playback, graphical client only |

The full text of each license ships with the official distribution of the
dependency in question, fetched by the script above.

## Scope

`libsodium` and `SQLite` are linked into the server, the CLI client, and
the graphical client. `Dear ImGui`, `GLFW` and `miniaudio` are linked only
into the graphical client — neither the server nor the CLI client depends
on them, and their absence doesn't prevent building those.
