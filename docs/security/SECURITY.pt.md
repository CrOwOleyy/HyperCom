# Política de segurança

[English](../../SECURITY.md) · [Français](SECURITY.fr.md) · [中文](SECURITY.zh.md) · [हिन्दी](SECURITY.hi.md) · [Español](SECURITY.es.md) · [العربية](SECURITY.ar.md) · [বাংলা](SECURITY.bn.md) · **Português** · [Русский](SECURITY.ru.md) · [日本語](SECURITY.ja.md)

O Hypercom é mantido por uma pequena equipe, fora de qualquer ambiente
profissional. Este documento diz honestamente o que isso implica.

## Relatar uma vulnerabilidade

**Não abra uma issue pública para uma vulnerabilidade de segurança.** Use
o [relato privado de vulnerabilidades do GitHub](../../../../security/advisories/new)
deste repositório (aba *Security* → *Report a vulnerability*): o relato
só fica visível para os mantenedores até que uma correção seja publicada.

Descreva:

- o arquivo e a função envolvidos, se possível;
- as condições exatas para reproduzir o problema;
- o impacto concreto (o que um atacante ganha, não só "isso parece
  errado").

## O que está no escopo

Qualquer falha que torne falsa uma das garantias listadas em
[docs/THREAT_MODEL.md](../THREAT_MODEL.md) — por exemplo: um DM legível
sem a chave do destinatário, um contorno da autenticação por assinatura,
uma injeção SQL, uma falha acionável remotamente sem autenticação, um
vazamento de segredo pela memória.

## O que não é uma vulnerabilidade

O `THREAT_MODEL.md` documenta limitações **aceitas**, não descuidos: o
servidor vê quem escreve para quem, "só amigos" não é criptografia, um
operador de servidor malicioso pode mentir para seus próprios usuários.
Relatos sobre esses pontos específicos serão fechados apontando para
esse documento, em vez de tratados como um bug.

## Versões cobertas

O Hypercom ainda não tem uma versão estável numerada: só a branch `main`
recebe correções de segurança.

## Tempo de resposta

Sem SLA formal — é um projeto de duas pessoas, não uma empresa. Na
prática: confirmação de recebimento em uma semana, correção ou plano de
ação comunicado antes que o relato se torne público.
