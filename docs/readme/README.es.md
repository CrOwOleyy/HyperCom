# HyperCom

[English](../../README.md) · [Français](README.fr.md) · [中文](README.zh.md) · [हिन्दी](README.hi.md) · **Español** · [العربية](README.ar.md) · [বাংলা](README.bn.md) · [Português](README.pt.md) · [Русский](README.ru.md) · [日本語](README.ja.md)

Red social descentralizada y cifrada de extremo a extremo: foros
comunitarios al estilo Reddit, perfiles al estilo MySpace, mensajes
privados que ningún servidor puede leer. Construida desde cero en C++20,
sin TLS ni dependencias web — un protocolo Noise escrito a mano sobre
libsodium.

Cada comunidad aloja su propio servidor, como en Discord, en lugar de un
servicio central único. Una sola identidad maestra permite unirse a
tantos servidores como se quiera, cada uno con una identidad separada y
sin correlación entre sí.

Documentación completa: [ARCHITECTURE.md](../../ARCHITECTURE.md) (cómo
encajan las piezas), [BRIEF.md](../../BRIEF.md) (decisiones de diseño),
[docs/ADMIN.md](../ADMIN.md) (administración del servidor),
[docs/THREAT_MODEL.md](../THREAT_MODEL.md) (qué está protegido y qué no),
[COMMANDS.md](../../COMMANDS.md) (referencia completa de comandos).

## Requisitos

- CMake ≥ 3.20, un compilador C++20 (MSVC en Windows, GCC/Clang en Linux)
- libsodium, SQLite y Dear ImGui: se descargan una vez mediante un
  script, nunca automáticamente por CMake (BRIEF.md 15)

**El servidor solo compila en Linux/WSL** (depende de epoll y signalfd).
El cliente — CLI e interfaz gráfica — compila tanto en Windows como en
Linux.

## Lado del servidor (Linux / WSL)

```bash
./scripts/fetch_third_party.sh
cmake -S . -B build/linux
cmake --build build/linux -j
```

Configuración inicial:

```bash
./build/linux/bin/hypercom_keygen server keys/server_static.key
chmod 600 keys/server_static.key
```

Crear `hypercom.conf` en la raíz del repositorio (todas las opciones
están documentadas en `docs/ADMIN.md §1-2`):

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

Luego:

```bash
./build/linux/bin/hypercom_server hypercom.conf
```

Al arrancar, el servidor imprime su clave pública y escribe
`run/hypercom-connect.txt`. Ese archivo — o simplemente la clave que
contiene — es lo que se entrega a alguien que se une al servidor, por un
canal de confianza, nunca haciendo que lo obtenga del propio servidor.

Administrar un servidor en ejecución (sesiones, MOTD, reportes,
baneos): `docs/ADMIN.md §7`.

## Lado del cliente (Windows o Linux)

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

Cargar el entorno una vez compilado — esto expone los atajos `hserver`,
`hgui` y `hcli`:

```powershell
cp env.example.ps1 env.ps1   # una sola vez, luego editar con tus valores
. .\env.ps1
```

Unirse a un servidor con el enlace de invitación recibido
(`hypercom://host:puerto#clave`):

```
hcli server-add hypercom://203.0.113.7:7717#447a6def... mi-servidor
hcli --server mi-servidor whoami
```

O directamente con el archivo de conexión que entregó el administrador:

```
hcli --connect-file hypercom-connect.txt whoami
```

Lanzar la interfaz gráfica:

```
hgui
```

Una segunda cuenta, para probar dos identidades en paralelo localmente:

```
hgui --identity cuenta2.key
```

## Pruebas

```bash
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

Detalle de las suites, compilaciones con sanitizers y la secuencia de
bienvenida del cliente: ver [COMMANDS.md](../../COMMANDS.md).
