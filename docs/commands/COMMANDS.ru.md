# Команды

[English](../../COMMANDS.md) · [Français](COMMANDS.fr.md) · [中文](COMMANDS.zh.md) · [हिन्दी](COMMANDS.hi.md) · [Español](COMMANDS.es.md) · [العربية](COMMANDS.ar.md) · [বাংলা](COMMANDS.bn.md) · [Português](COMMANDS.pt.md) · **Русский** · [日本語](COMMANDS.ja.md)

## Локальная конфигурация

`env.ps1` (Windows) и `env.sh` (Linux/WSL) не версионируются: они
содержат вашу локальную парольную фразу и открытый ключ вашего сервера.
Первое использование:

```
cp env.example.ps1 env.ps1   # или env.example.sh -> env.sh
# затем отредактировать env.ps1 / env.sh своими значениями
```

## Администрирование сервера

Сервер открывает локальный сокет (`run/hypercom-admin.sock` по
умолчанию), на который `hypercom_adminctl` отправляет команды. Никакого
C++ писать не нужно, никакого SQL вводить не нужно.

```
hypercom_adminctl help
hypercom_adminctl stats
hypercom_adminctl sessions
hypercom_adminctl sessions close 12
hypercom_adminctl motd set "техобслуживание в субботу в 14:00"
hypercom_adminctl motd clear
hypercom_adminctl backup backups/hypercom.db
```

Если сокет не по умолчанию:

```
hypercom_adminctl --socket /var/run/hypercom-admin.sock stats
```

Подробности и гарантии безопасности: docs/ADMIN.md §7.

## Запуск тестов

После каждого изменения — пересобрать и запустить набор:

```
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

Девять наборов:

| Набор | Что покрывает |
|---|---|
| `protocol_parsing_test` | границы ридера, лимиты, валидация UTF-8, кадрирование |
| `crypto_round_trip_test` | рукопожатие, зашифрованные личные сообщения, хранилище ключей |
| `noise_official_vectors_test` | побайтовое сравнение с официальным вектором Noise |
| `message_roundtrip_session_test` | сессионные сообщения: hello, auth, ping, MOTD, status |
| `message_roundtrip_content_test` | форумы, посты, комментарии, ветки |
| `message_roundtrip_social_test` | аккаунт, prekeys, профили, друзья, топ-8, личные сообщения |
| `multi_server_identity_test` | личности по серверам, панель серверов, жизненный цикл слотов |
| `fuzz_corpus_replay_test` | повтор корпуса фаззинга, без libFuzzer |
| `reconnection_test` | полное повторное рукопожатие на том же объекте (только POSIX) |

С санитайзерами:

```
cmake -S . -B build-asan -DHYPERCOM_SANITIZER=address,undefined
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```

TSAN запускается из отдельной директории (несовместим с ASAN). Под WSL
нужно отключить ASLR, иначе он откажется запускаться:

```
setarch -R ctest --test-dir build-tsan --output-on-failure
```

Один тест, чтобы увидеть детали:

```
build\windows\bin\RelWithDebInfo\noise_official_vectors_test.exe
```

## На Windows (PowerShell)

### 1. Установить зависимости
```
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\scripts\fetch_third_party.ps1
```

### 2. Настроить сборку

```
cmake -S . -B build/windows
```

### 3. Собрать проект
```
cmake --build build/windows --config RelWithDebInfo -j
```

### 4. Запустить проект
#### Загрузить окружение
```
. .\env.ps1
```

#### Запустить сервер локально в терминале 1
```
hserver
```

#### Запустить графический клиент (GUI)
```
hgui
```

#### Запустить 2-й клиент (для теста со 2-м аккаунтом)
```
hgui --identity account2.key
```

## Приветственная последовательность

Она проигрывается **только при создании аккаунта**, никогда при
последующих подключениях. Чтобы пересмотреть её без создания
одноразового аккаунта:

```
hgui --replay-intro
```

Или начав с новой личности — осторожно, файл не должен уже
существовать, иначе аккаунт уже зарегистрирован и intro не
запускается:

```
hgui --identity new_account.key
```

Последовательность, привязанная к реальной длительности `menu.mp3`
(~12,5 с):

| Момент | Что происходит |
|---|---|
| 0 → 6 с | Стеклянная карточка по центру, «Добро пожаловать в пространство HyperCom.» |
| 6 с → конец | Три колонки поднимаются одна за другой, пузырьками |
| затем | Обычный интерфейс, больше никакой анимации |

Замена `menu.mp3` пересчитывает анимацию автоматически: длительность
читается из файла. CMake копирует его рядом с исполняемым файлом при
каждой сборке.

Нет звука? Это никогда не блокирует — intro проигрывается идентично по
часам. Клиент печатает причину при запуске:

```
intro: music, duration used 12.5268 s
```

## На Linux / WSL

### 1. Установить зависимости
```
./scripts/fetch_third_party.sh
```

### 2. Настроить сборку
```
cmake -S . -B build/linux
```

### 3. Собрать проект
```
cmake --build build/linux -j
```

### 4. Запустить проект
*(Загружает сокращения `hgui`, `hcli` и `hserver`)*

#### Запустить сервер локально (терминал 1)
```
hserver
```

#### Загрузить окружение (терминал 2)
```
source env.sh
```

#### Запустить графический клиент (GUI) (терминал 2)
```
hgui
```
