# Commands

**English** · [Français](docs/commands/COMMANDS.fr.md) · [中文](docs/commands/COMMANDS.zh.md) · [हिन्दी](docs/commands/COMMANDS.hi.md) · [Español](docs/commands/COMMANDS.es.md) · [العربية](docs/commands/COMMANDS.ar.md) · [বাংলা](docs/commands/COMMANDS.bn.md) · [Português](docs/commands/COMMANDS.pt.md) · [Русский](docs/commands/COMMANDS.ru.md) · [日本語](docs/commands/COMMANDS.ja.md)

## Local configuration

`env.ps1` (Windows) and `env.sh` (Linux/WSL) aren't versioned: they hold
your local passphrase and your server's public key. First use:

```
cp env.example.ps1 env.ps1   # or env.example.sh -> env.sh
# then edit env.ps1 / env.sh with your own values
```

## Administering the server

The server opens a local socket (`run/hypercom-admin.sock` by default) that
`hypercom_adminctl` sends commands to. No C++ to write, no SQL to type.

```
hypercom_adminctl help
hypercom_adminctl stats
hypercom_adminctl sessions
hypercom_adminctl sessions close 12
hypercom_adminctl motd set "maintenance Saturday 2pm"
hypercom_adminctl motd clear
hypercom_adminctl backup backups/hypercom.db
```

If the socket isn't at the default path:

```
hypercom_adminctl --socket /var/run/hypercom-admin.sock stats
```

Details and security guarantees: docs/ADMIN.md §7.

## Running the tests

After every change, rebuild then run the suite:

```
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

Nine suites:

| Suite | What it covers |
|---|---|
| `protocol_parsing_test` | reader bounds, caps, UTF-8 validation, framing |
| `crypto_round_trip_test` | handshake, encrypted DMs, keystore |
| `noise_official_vectors_test` | byte-for-byte comparison against an official Noise vector |
| `message_roundtrip_session_test` | session messages: hello, auth, ping, MOTD, status |
| `message_roundtrip_content_test` | forums, posts, comments, threads |
| `message_roundtrip_social_test` | account, prekeys, profiles, friends, top 8, DMs |
| `multi_server_identity_test` | per-server identities, the server bar, slot lifecycle |
| `fuzz_corpus_replay_test` | replays the fuzzing corpus, without libFuzzer |
| `reconnection_test` | full re-handshake on the same object (POSIX only) |

Under sanitizers:

```
cmake -S . -B build-asan -DHYPERCOM_SANITIZER=address,undefined
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```

TSAN runs from a separate directory (incompatible with ASAN). Under WSL,
ASLR needs to be disabled or it refuses to start:

```
setarch -R ctest --test-dir build-tsan --output-on-failure
```

A single test, to see the detail:

```
build\windows\bin\RelWithDebInfo\noise_official_vectors_test.exe
```

## On Windows (PowerShell)

### 1. Install dependencies
```
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\scripts\fetch_third_party.ps1
```

### 2. Configure the build

```
cmake -S . -B build/windows
```

### 3. Build the project
```
cmake --build build/windows --config RelWithDebInfo -j
```

### 4. Run the project
#### Load the environment
```
. .\env.ps1
```

#### Launch the server locally in terminal 1
```
hserver
```

#### Launch the graphical client (GUI)
```
hgui
```

#### Launch a 2nd client (to test with a 2nd account)
```
hgui --identity account2.key
```

## The welcome sequence

It only plays **when an account is created**, never on later connections.
To replay it without creating a throwaway account:

```
hgui --replay-intro
```

Or by starting from a fresh identity — careful, the file must not already
exist, otherwise the account is already registered and the intro never
triggers:

```
hgui --identity fresh_account.key
```

Sequence, timed to the actual duration of `menu.mp3` (~12.5s):

| Moment | What happens |
|---|---|
| 0 → 6s | Centered glass card, "Welcome to the HyperCom space." |
| 6s → end | The three columns rise one after another, as bubbles |
| after | Normal interface, no more animation |

Replacing `menu.mp3` recalibrates the animation on its own: the duration
is read from the file. CMake copies it next to the executable on every
build.

No sound? Never a blocker — the intro plays out identically on the clock.
The client prints the reason at startup:

```
intro: music, duration used 12.5268 s
```

## On Linux / WSL

### 1. Install dependencies
```
./scripts/fetch_third_party.sh
```

### 2. Configure the build
```
cmake -S . -B build/linux
```

### 3. Build the project
```
cmake --build build/linux -j
```

### 4. Run the project
*(Loads the `hgui`, `hcli` and `hserver` shortcuts)*

#### Launch the server locally (terminal 1)
```
hserver
```

#### Load the environment (terminal 2)
```
source env.sh
```

#### Launch the graphical client (GUI) (terminal 2)
```
hgui
```
