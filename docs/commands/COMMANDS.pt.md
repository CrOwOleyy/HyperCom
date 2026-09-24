# Comandos

[English](../../COMMANDS.md) · [Français](COMMANDS.fr.md) · [中文](COMMANDS.zh.md) · [हिन्दी](COMMANDS.hi.md) · [Español](COMMANDS.es.md) · [العربية](COMMANDS.ar.md) · [বাংলা](COMMANDS.bn.md) · **Português** · [Русский](COMMANDS.ru.md) · [日本語](COMMANDS.ja.md)

## Configuração local

`env.ps1` (Windows) e `env.sh` (Linux/WSL) não são versionados: eles
contêm sua passphrase local e a chave pública do seu servidor. Primeiro
uso:

```
cp env.example.ps1 env.ps1   # ou env.example.sh -> env.sh
# depois edite env.ps1 / env.sh com seus próprios valores
```

## Administrar o servidor

O servidor abre um socket local (`run/hypercom-admin.sock` por padrão)
para o qual o `hypercom_adminctl` envia comandos. Nada de C++ para
escrever, nada de SQL para digitar.

```
hypercom_adminctl help
hypercom_adminctl stats
hypercom_adminctl sessions
hypercom_adminctl sessions close 12
hypercom_adminctl motd set "manutenção sábado 14h"
hypercom_adminctl motd clear
hypercom_adminctl backup backups/hypercom.db
```

Se o socket não estiver no caminho padrão:

```
hypercom_adminctl --socket /var/run/hypercom-admin.sock stats
```

Detalhes e garantias de segurança: docs/ADMIN.md §7.

## Rodando os testes

Depois de cada mudança, recompile e rode a suíte:

```
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

Nove suítes:

| Suíte | O que cobre |
|---|---|
| `protocol_parsing_test` | limites do leitor, tetos, validação UTF-8, framing |
| `crypto_round_trip_test` | handshake, DMs criptografados, keystore |
| `noise_official_vectors_test` | comparação byte a byte com um vetor oficial do Noise |
| `message_roundtrip_session_test` | mensagens de sessão: hello, auth, ping, MOTD, status |
| `message_roundtrip_content_test` | fóruns, posts, comentários, tópicos |
| `message_roundtrip_social_test` | conta, prekeys, perfis, amigos, top 8, DMs |
| `multi_server_identity_test` | identidades por servidor, barra de servidores, ciclo de vida dos slots |
| `fuzz_corpus_replay_test` | reproduz o corpus de fuzzing, sem libFuzzer |
| `reconnection_test` | re-handshake completo sobre o mesmo objeto (só POSIX) |

Sob sanitizers:

```
cmake -S . -B build-asan -DHYPERCOM_SANITIZER=address,undefined
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```

O TSAN roda num diretório separado (incompatível com ASAN). No WSL é
preciso desativar o ASLR, senão ele se recusa a iniciar:

```
setarch -R ctest --test-dir build-tsan --output-on-failure
```

Um único teste, para ver o detalhe:

```
build\windows\bin\RelWithDebInfo\noise_official_vectors_test.exe
```

## No Windows (PowerShell)

### 1. Instalar as dependências
```
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\scripts\fetch_third_party.ps1
```

### 2. Configurar o build

```
cmake -S . -B build/windows
```

### 3. Compilar o projeto
```
cmake --build build/windows --config RelWithDebInfo -j
```

### 4. Rodar o projeto
#### Carregar o ambiente
```
. .\env.ps1
```

#### Lançar o servidor localmente no terminal 1
```
hserver
```

#### Lançar o cliente com interface gráfica (GUI)
```
hgui
```

#### Lançar um 2º cliente (para testar com uma 2ª conta)
```
hgui --identity conta2.key
```

## A sequência de boas-vindas

Ela só toca **na criação de uma conta**, nunca nas conexões seguintes.
Para revê-la sem criar uma conta descartável:

```
hgui --replay-intro
```

Ou partindo de uma identidade nova — atenção, o arquivo não pode já
existir, senão a conta já está registrada e a intro não é disparada:

```
hgui --identity conta_nova.key
```

Sequência, calibrada pela duração real de `menu.mp3` (~12,5 s):

| Momento | O que acontece |
|---|---|
| 0 → 6 s | Cartão de vidro centralizado, "Bem-vindo ao espaço HyperCom." |
| 6 s → fim | As três colunas sobem uma após a outra, como bolhas |
| depois | Interface normal, sem mais animação |

Substituir `menu.mp3` recalibra a animação sozinho: a duração é lida do
arquivo. O CMake o copia ao lado do executável a cada build.

Sem som? Nunca é bloqueante — a intro toca do mesmo jeito, no relógio. O
cliente mostra o motivo ao iniciar:

```
intro: música, duração usada 12.5268 s
```

## No Linux / WSL

### 1. Instalar as dependências
```
./scripts/fetch_third_party.sh
```

### 2. Configurar o build
```
cmake -S . -B build/linux
```

### 3. Compilar o projeto
```
cmake --build build/linux -j
```

### 4. Rodar o projeto
*(Carrega os atalhos `hgui`, `hcli` e `hserver`)*

#### Lançar o servidor localmente (terminal 1)
```
hserver
```

#### Carregar o ambiente (terminal 2)
```
source env.sh
```

#### Lançar o cliente com interface gráfica (GUI) (terminal 2)
```
hgui
```
