# HyperCom

[English](../../README.md) · [Français](README.fr.md) · [中文](README.zh.md) · [हिन्दी](README.hi.md) · [Español](README.es.md) · [العربية](README.ar.md) · [বাংলা](README.bn.md) · **Português** · [Русский](README.ru.md) · [日本語](README.ja.md)

Rede social descentralizada e criptografada de ponta a ponta: fóruns
comunitários ao estilo Reddit, perfis ao estilo MySpace, mensagens
privadas que nenhum servidor consegue ler. Construída do zero em C++20,
sem TLS nem dependências web — um protocolo Noise escrito à mão sobre a
libsodium.

Cada comunidade hospeda seu próprio servidor, à moda do Discord, em vez
de um serviço central único. Uma única identidade mestra permite entrar
em quantos servidores quiser, cada um com uma identidade separada e sem
correlação entre si.

Documentação completa: [ARCHITECTURE.md](../../ARCHITECTURE.md) (como as
peças se encaixam), [docs/ADMIN.md](../ADMIN.md) (administração do servidor),
[docs/THREAT_MODEL.md](../THREAT_MODEL.md) (o que é protegido e o que
não é), [SECURITY.md](../../SECURITY.md) (relatar uma vulnerabilidade),
[COMMANDS.md](../../COMMANDS.md) (referência completa de comandos).

## Requisitos

- CMake ≥ 3.20, um compilador C++20 (MSVC no Windows, GCC/Clang no Linux)
- libsodium, SQLite e Dear ImGui: baixados uma vez por um script, nunca
  automaticamente pelo CMake

**O servidor só compila em Linux/WSL** (depende de epoll e signalfd). O
cliente — CLI e interface gráfica — compila tanto no Windows quanto no
Linux.

## Lado do servidor (Linux / WSL)

```bash
./scripts/fetch_third_party.sh
cmake -S . -B build/linux
cmake --build build/linux -j
```

Configuração inicial:

```bash
./build/linux/bin/hypercom_keygen server keys/server_static.key
chmod 600 keys/server_static.key
```

Criar `hypercom.conf` na raiz do repositório (todas as opções estão
documentadas em `docs/ADMIN.md §1-2`):

```ini
[server]
registration_open = true

[clearnet]
enabled      = true
bind_address = 0.0.0.0
port         = 7717

[onion]
enabled = false

[limits]
max_connections           = 512
handshake_timeout_seconds = 10
idle_timeout_seconds      = 0

[paths]
database     = hypercom.db
server_key   = keys/server_static.key
admin_socket = run/hypercom-admin.sock
```

Depois:

```bash
./build/linux/bin/hypercom_server hypercom.conf
```

Ao iniciar, o servidor imprime sua chave pública e grava
`run/hypercom-connect.txt`. Esse arquivo — ou apenas a chave contida nele
— é o que se entrega a alguém que vai entrar no servidor, por um canal de
confiança, nunca deixando que a pessoa o obtenha a partir do próprio
servidor.

Administrar um servidor em execução (sessões, MOTD, denúncias,
banimentos): `docs/ADMIN.md §7`.

## Lado do cliente (Windows ou Linux)

```powershell
.\scripts\fetch_third_party.ps1
cmake -S . -B build/windows
cmake --build build/windows --config RelWithDebInfo -j
```

```bash
./scripts/fetch_third_party.sh
cmake -S . -B build/linux
cmake --build build/linux -j
```

Carregar o ambiente depois de compilado — isso disponibiliza os atalhos
`hserver`, `hgui` e `hcli`:

```powershell
cp env.example.ps1 env.ps1   # uma unica vez, depois editar com seus valores
. .\env.ps1
```

Entrar em um servidor usando o link de convite recebido
(`hypercom://host:porta#chave`):

```
hcli server-add hypercom://203.0.113.7:7717#447a6def... meu-servidor
hcli --server meu-servidor whoami
```

Ou diretamente com o arquivo de conexão que o administrador entregou:

```
hcli --connect-file hypercom-connect.txt whoami
```

Iniciar a interface gráfica:

```
hgui
```

Uma segunda conta, para testar duas identidades lado a lado localmente:

```
hgui --identity conta2.key
```

## Testes

```bash
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

Detalhes das suítes, builds com sanitizers e a sequência de boas-vindas
do cliente: ver [COMMANDS.md](../../COMMANDS.md).

## Status

Isto é um protótipo: espere bugs, e algumas partes ainda estão um
pouco brutas. Encontrou um? Abra uma issue, ou mande um pull request
se já tiver uma correção. Para qualquer outra coisa, escreva para
[leyy@pepepak.fr](mailto:leyy@pepepak.fr).
