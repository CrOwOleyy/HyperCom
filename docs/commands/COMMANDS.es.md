# Comandos

[English](../../COMMANDS.md) · [Français](COMMANDS.fr.md) · [中文](COMMANDS.zh.md) · [हिन्दी](COMMANDS.hi.md) · **Español** · [العربية](COMMANDS.ar.md) · [বাংলা](COMMANDS.bn.md) · [Português](COMMANDS.pt.md) · [Русский](COMMANDS.ru.md) · [日本語](COMMANDS.ja.md)

## Configuración local

`env.ps1` (Windows) y `env.sh` (Linux/WSL) no están versionados: contienen
tu passphrase local y la clave pública de tu servidor. Primer uso:

```
cp env.example.ps1 env.ps1   # o env.example.sh -> env.sh
# luego editar env.ps1 / env.sh con tus propios valores
```

## Administrar el servidor

El servidor abre un socket local (`run/hypercom-admin.sock` por defecto) al
que `hypercom_adminctl` envía comandos. Nada de C++ que escribir, nada de
SQL que teclear.

```
hypercom_adminctl help
hypercom_adminctl stats
hypercom_adminctl sessions
hypercom_adminctl sessions close 12
hypercom_adminctl motd set "mantenimiento sábado 14h"
hypercom_adminctl motd clear
hypercom_adminctl backup backups/hypercom.db
```

Si el socket no está en la ruta por defecto:

```
hypercom_adminctl --socket /var/run/hypercom-admin.sock stats
```

Detalles y garantías de seguridad: docs/ADMIN.md §7.

## Ejecutar las pruebas

Tras cada cambio, recompilar y luego correr la suite:

```
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

Nueve suites:

| Suite | Qué cubre |
|---|---|
| `protocol_parsing_test` | límites del lector, topes, validación UTF-8, framing |
| `crypto_round_trip_test` | handshake, DMs cifrados, keystore |
| `noise_official_vectors_test` | comparación byte a byte con un vector oficial de Noise |
| `message_roundtrip_session_test` | mensajes de sesión: hello, auth, ping, MOTD, status |
| `message_roundtrip_content_test` | foros, posts, comentarios, hilos |
| `message_roundtrip_social_test` | cuenta, prekeys, perfiles, amigos, top 8, DMs |
| `multi_server_identity_test` | identidades por servidor, barra de servidores, ciclo de vida de los slots |
| `fuzz_corpus_replay_test` | repite el corpus de fuzzing, sin libFuzzer |
| `reconnection_test` | re-handshake completo sobre el mismo objeto (solo POSIX) |

Bajo sanitizers:

```
cmake -S . -B build-asan -DHYPERCOM_SANITIZER=address,undefined
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```

TSAN se ejecuta desde un directorio separado (incompatible con ASAN). En
WSL hay que desactivar el ASLR, si no se niega a arrancar:

```
setarch -R ctest --test-dir build-tsan --output-on-failure
```

Una sola prueba, para ver el detalle:

```
build\windows\bin\RelWithDebInfo\noise_official_vectors_test.exe
```

## En Windows (PowerShell)

### 1. Instalar las dependencias
```
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\scripts\fetch_third_party.ps1
```

### 2. Configurar el build

```
cmake -S . -B build/windows
```

### 3. Compilar el proyecto
```
cmake --build build/windows --config RelWithDebInfo -j
```

### 4. Ejecutar el proyecto
#### Cargar el entorno
```
. .\env.ps1
```

#### Lanzar el servidor localmente en la terminal 1
```
hserver
```

#### Lanzar el cliente con interfaz gráfica (GUI)
```
hgui
```

#### Lanzar un 2º cliente (para probar con una 2ª cuenta)
```
hgui --identity cuenta2.key
```

## La secuencia de bienvenida

Solo se reproduce **al crear una cuenta**, nunca en conexiones
posteriores. Para verla de nuevo sin crear una cuenta desechable:

```
hgui --replay-intro
```

O partiendo de una identidad nueva — cuidado, el archivo no debe existir
ya, si no la cuenta ya está registrada y la intro no se dispara:

```
hgui --identity cuenta_nueva.key
```

Desarrollo, calibrado con la duración real de `menu.mp3` (~12,5 s):

| Momento | Qué ocurre |
|---|---|
| 0 → 6 s | Tarjeta de cristal centrada, "Bienvenido al espacio HyperCom." |
| 6 s → fin | Las tres columnas suben una tras otra, como burbujas |
| después | Interfaz normal, sin más animación |

Reemplazar `menu.mp3` recalibra la animación solo: la duración se lee del
archivo. CMake lo copia junto al ejecutable en cada build.

¿Sin sonido? Nunca bloquea — la intro se reproduce igual sobre el reloj.
El cliente muestra el motivo al arrancar:

```
intro: música, duración usada 12.5268 s
```

## En Linux / WSL

### 1. Instalar las dependencias
```
./scripts/fetch_third_party.sh
```

### 2. Configurar el build
```
cmake -S . -B build/linux
```

### 3. Compilar el proyecto
```
cmake --build build/linux -j
```

### 4. Ejecutar el proyecto
*(Carga los atajos `hgui`, `hcli` y `hserver`)*

#### Lanzar el servidor localmente (terminal 1)
```
hserver
```

#### Cargar el entorno (terminal 2)
```
source env.sh
```

#### Lanzar el cliente con interfaz gráfica (GUI) (terminal 2)
```
hgui
```
