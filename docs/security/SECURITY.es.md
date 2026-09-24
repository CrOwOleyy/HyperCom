# Política de seguridad

[English](../../SECURITY.md) · [Français](SECURITY.fr.md) · [中文](SECURITY.zh.md) · [हिन्दी](SECURITY.hi.md) · **Español** · [العربية](SECURITY.ar.md) · [বাংলা](SECURITY.bn.md) · [Português](SECURITY.pt.md) · [Русский](SECURITY.ru.md) · [日本語](SECURITY.ja.md)

Hypercom lo mantiene un equipo pequeño, fuera de cualquier entorno
profesional. Este documento dice honestamente lo que eso implica.

## Reportar una vulnerabilidad

**No abrir un issue público para una vulnerabilidad de seguridad.** Usar el
[reporte privado de vulnerabilidades de GitHub](../../../../security/advisories/new)
de este repositorio (pestaña *Security* → *Report a vulnerability*): el
reporte solo es visible para los mantenedores hasta que se publique una
corrección.

Describir:

- el archivo y la función involucrados, si es posible;
- las condiciones exactas para reproducir el problema;
- el impacto concreto (qué obtiene un atacante, no solo "esto se ve mal").

## Qué está dentro del alcance

Cualquier falla que invalide una de las garantías listadas en
[docs/THREAT_MODEL.md](../THREAT_MODEL.md) — por ejemplo: un DM legible
sin la clave del destinatario, una omisión de la autenticación basada en
firma, una inyección SQL, un fallo provocable remotamente sin
autenticación, una fuga de un secreto en memoria.

## Qué no es una vulnerabilidad

`THREAT_MODEL.md` documenta limitaciones **aceptadas**, no descuidos: el
servidor ve quién le escribe a quién, "solo amigos" no es cifrado, un
operador de servidor malicioso puede mentirle a sus propios usuarios. Los
reportes sobre estos puntos específicos se cerrarán señalando ese
documento en lugar de tratarse como un error.

## Versiones cubiertas

Hypercom todavía no tiene una versión estable numerada: solo la rama
`main` recibe correcciones de seguridad.

## Tiempo de respuesta

Sin SLA formal — es un proyecto de dos personas, no una empresa. En la
práctica: confirmación de recepción en una semana, una corrección o un
plan de acción comunicado antes de que el reporte se haga público.
