# Architecture

[English](../../ARCHITECTURE.md) · [Français](ARCHITECTURE.fr.md) · [中文](ARCHITECTURE.zh.md) · [हिन्दी](ARCHITECTURE.hi.md) · **Español** · [العربية](ARCHITECTURE.ar.md) · [বাংলা](ARCHITECTURE.bn.md) · [Português](ARCHITECTURE.pt.md) · [Русский](ARCHITECTURE.ru.md) · [日本語](ARCHITECTURE.ja.md)

Este documento explica cómo encajan las piezas: qué ocurre entre el
momento en que un cliente se conecta y el momento en que un mensaje
aparece en un hilo del foro o en una bandeja de mensajes privados. Para
el detalle de un tema concreto, los demás documentos profundizan más:
[PROTOCOL.md](../PROTOCOL.md) para el formato de las tramas,
[SCHEMA.md](../SCHEMA.md) para la base de datos,
[THREAT_MODEL.md](../THREAT_MODEL.md) para lo que está protegido y lo
que no, [ADMIN.md](../ADMIN.md) para administrar un servidor.

## El servidor nunca ejecuta nada en paralelo

`server_runtime::run_until_stopped` es un único bucle `epoll`. No hay un
hilo por conexión, ni un pool de hilos. Cada conexión, cada consulta a
la base de datos, cada operación criptográfica pasa una tras otra por
ese mismo bucle, y no existe ni un solo mutex en todo `server/` —
no hay nada que proteger porque nada se ejecuta de forma concurrente.
Dos peticiones tocando la misma fila al mismo tiempo, un contador de
límite de tasa corrompido por una escritura concurrente: toda esa
familia de errores simplemente no tiene dónde ocurrir. La contrapartida
aparece como un techo en el rendimiento máximo, pero para un servidor
que atiende a una comunidad y no a un servicio con millones de
usuarios, ese bucle único nunca se convierte en el cuello de botella.

## Del socket al hilo del foro

Un mensaje entrante atraviesa estas capas, en este orden:

```
socket TCP
  → búfer de bytes crudos (connection_socket)
  → eliminación del prefijo de longitud (extract_length_prefixed_message)
  → handshake Noise en curso, o descifrado si ya está establecido
  → decodificación de la cabecera de trama (frame_codec)
  → limitación de tasa (por dirección, luego por identidad)
  → enrutamiento por familia de mensaje (request_router)
  → handler (account_handler, forum_handler, dm_handler, ...)
  → repositorio (post_repository, dm_repository, ...)
  → SQLite
```

La primera bifurcación — handshake o trama de aplicación — se decide en
`connection_processor.cpp`. Mientras `channel.is_established()` siga
devolviendo falso, cada mensaje que llega hace avanzar el handshake
Noise en lugar de tratarse como una petición; una vez establecido el
canal, todo lo que llega se descifra y se interpreta como una trama.

El enrutamiento está dividido en familias (sesión, contenido, social,
DM, reportes) en lugar de un único `switch` gigante sobre todos los
tipos de mensaje: las reglas de estilo del proyecto limitan una función
a sesenta líneas, y un switch que cubriera la veintena de tipos de
mensaje existentes las superaría con creces. `route_message` prueba
cada familia por turno y se detiene en cuanto una de ellas reconoce el
tipo.

Cada handler solo conoce su propia tarea — `handle_dm_send_request` no
tiene ni idea de que existe la tabla `posts`. Lo que comparten es el
`handler_context` (configuración, logger, conexión a la base de datos,
conexión del cliente) y los repositorios, que son el único lugar del
código que toca SQLite directamente. Un handler que construyera su
propia consulta SQL sería una señal de alarma llegados a este punto.

## El canal cifrado

El transporte no es TLS — no hay ningún cliente web al que satisfacer,
y TLS arrastra X.509 y las autoridades de certificación, dos cosas para
las que este proyecto no tiene ningún uso. En su lugar: Noise NK sobre
libsodium. El cliente ya conoce de antemano la clave pública estática
del servidor (fijada en la primera conexión, o leída desde un archivo
de conexión compartido); el handshake simplemente falla si un servidor
suplantado intenta responder en su lugar.

Una vez completado el handshake (el paso que el protocolo Noise llama
`Split`), cada sentido de la comunicación obtiene su propia clave y su
propio contador de nonce dentro de `noise_transport`. Un mensaje del
cliente y un mensaje del servidor nunca pueden compartir el mismo
nonce — si lo hicieran, ChaCha20-Poly1305 dejaría de ser seguro. Todo
lo que sigue, protocolo de aplicación incluido, existe en claro
únicamente en las dos máquinas de los extremos.

## Un cliente, varios servidores

El cliente se parece más a Discord que a Slack: una única ventana, una
barra de servidores al lado, y cada servidor conservando su propia
identidad. Una identidad compartida entre dos servidores sería un
identificador que dos administradores podrían cruzar para establecer
que se trata de la misma persona en ambos lados — algo que el proyecto
evita dando a cada servidor su propio par de claves, sin ningún vínculo
visible entre ellas.

Dos estructuras se reparten el estado:

- `app_state` contiene todo lo que pertenece a la aplicación como un
  todo: el idioma, la pestaña activa, la frase de contraseña mantenida
  en memoria durante la sesión (nunca escrita en disco).
- `server_slot` contiene todo lo que pertenece a *un* servidor: su
  conexión, su identidad, su sesión, y el estado de la vista de lo que
  esté mostrando.

Las pestañas viven en un `std::vector<std::unique_ptr<server_slot>>`,
nunca por valor. La razón se reduce a un detalle de implementación
fácil de romper por accidente: `client_session` guarda referencias
hacia `server_connection` y hacia la identidad de la pestaña. Si el
vector guardara `server_slot` por valor, un `push_back` que provocara
una realocación invalidaría esas referencias sin avisar a nadie — el
`unique_ptr` fija la dirección de la pestaña para siempre, así que
añadir un servidor nunca mueve los que ya existían.

## Dónde buscar qué

| Pregunta | Directorio |
|---|---|
| Cómo está estructurado un mensaje en el cable | `common/protocol/` |
| Cifrado, derivación de claves, DMs | `common/crypto/` |
| Bucle de red, límite de tasa, sesiones | `server/net/` |
| Acceso a SQLite | `server/db/` |
| Lógica de negocio por tipo de mensaje | `server/handlers/` |
| Comandos de administración (socket local) | `server/admin/` |
| Conexión, keystore, multiservidor | `client/net/`, `client/keystore/` |
| Interfaz ImGui | `client/ui/` |
