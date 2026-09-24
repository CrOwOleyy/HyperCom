# HyperCom

[English](../../README.md) · [Français](README.fr.md) · [中文](README.zh.md) · [हिन्दी](README.hi.md) · [Español](README.es.md) · [العربية](README.ar.md) · [বাংলা](README.bn.md) · [Português](README.pt.md) · **Русский** · [日本語](README.ja.md)

Децентрализованная социальная сеть со сквозным шифрованием: форумы в
духе Reddit, профили в духе MySpace, личные сообщения, которые не может
прочитать ни один сервер. Написана с нуля на C++20, без TLS и веб-
зависимостей — протокол Noise, реализованный вручную поверх libsodium.

Каждое сообщество размещает собственный сервер, как в Discord, вместо
единого центрального сервиса. Одна главная личность позволяет
присоединяться к любому количеству серверов, и на каждом из них — своя
отдельная личность, не связанная с остальными.

Полная документация: [ARCHITECTURE.md](../../ARCHITECTURE.md) (как всё
устроено вместе), [docs/ADMIN.md](../ADMIN.md) (администрирование сервера),
[docs/THREAT_MODEL.md](../THREAT_MODEL.md) (что защищено, а что нет),
[SECURITY.md](../../SECURITY.md) (как сообщить об уязвимости),
[COMMANDS.md](../../COMMANDS.md) (полный справочник команд).

## Требования

- CMake ≥ 3.20, компилятор C++20 (MSVC на Windows, GCC/Clang на Linux)
- libsodium, SQLite и Dear ImGui: загружаются один раз через скрипт,
  никогда автоматически через CMake

**Сервер собирается только под Linux/WSL** (использует epoll и
signalfd). Клиент — CLI и графический интерфейс — собирается и под
Windows, и под Linux.

## Серверная часть (Linux / WSL)

```bash
./scripts/fetch_third_party.sh
cmake -S . -B build/linux
cmake --build build/linux -j
```

Первоначальная настройка:

```bash
./build/linux/bin/hypercom_keygen server keys/server_static.key
chmod 600 keys/server_static.key
```

Создать `hypercom.conf` в корне репозитория (все опции описаны в
`docs/ADMIN.md §1-2`):

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

Затем:

```bash
./build/linux/bin/hypercom_server hypercom.conf
```

При запуске сервер выводит свой открытый ключ и записывает
`run/hypercom-connect.txt`. Именно этот файл — или просто ключ из него —
передают тому, кто присоединяется к серверу, по доверенному каналу, и
никогда не позволяют получить его с самого сервера.

Администрирование работающего сервера (сессии, MOTD, жалобы, баны):
`docs/ADMIN.md §7`.

## Клиентская часть (Windows или Linux)

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

Загрузить окружение после сборки — это открывает доступ к сокращениям
`hserver`, `hgui` и `hcli`:

```powershell
cp env.example.ps1 env.ps1   # один раз, затем отредактировать своими значениями
. .\env.ps1
```

Присоединиться к серверу по полученной пригласительной ссылке
(`hypercom://хост:порт#ключ`):

```
hcli server-add hypercom://203.0.113.7:7717#447a6def... мой-сервер
hcli --server мой-сервер whoami
```

Или напрямую через файл подключения, переданный администратором:

```
hcli --connect-file hypercom-connect.txt whoami
```

Запустить графический интерфейс:

```
hgui
```

Второй аккаунт для локального тестирования двух личностей одновременно:

```
hgui --identity account2.key
```

## Тестирование

```bash
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

Подробности о наборах тестов, сборках с санитайзерами и приветственной
последовательности клиента: см. [COMMANDS.md](../../COMMANDS.md).
