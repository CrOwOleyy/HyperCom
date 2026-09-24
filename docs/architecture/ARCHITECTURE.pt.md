# Architecture

[English](../../ARCHITECTURE.md) · [Français](ARCHITECTURE.fr.md) · [中文](ARCHITECTURE.zh.md) · [हिन्दी](ARCHITECTURE.hi.md) · [Español](ARCHITECTURE.es.md) · [العربية](ARCHITECTURE.ar.md) · [বাংলা](ARCHITECTURE.bn.md) · **Português** · [Русский](ARCHITECTURE.ru.md) · [日本語](ARCHITECTURE.ja.md)

Este documento explica como as peças se encaixam: o que acontece entre
o momento em que um cliente se conecta e o momento em que uma mensagem
chega a um tópico do fórum ou a uma caixa de mensagens privadas. Para
detalhes de um tema específico, os outros documentos vão mais fundo:
[PROTOCOL.md](../PROTOCOL.md) para o formato das tramas,
[SCHEMA.md](../SCHEMA.md) para o banco de dados,
[THREAT_MODEL.md](../THREAT_MODEL.md) para o que está protegido e o que
não está, [ADMIN.md](../ADMIN.md) para administrar um servidor.

## O servidor nunca executa nada em paralelo

`server_runtime::run_until_stopped` é um único laço `epoll`. Nada de
thread por conexão, nada de pool. Cada conexão, cada consulta ao banco,
cada operação criptográfica passa uma após a outra por esse mesmo
laço, e não existe um único mutex em `server/` — não há nada a
proteger, já que nada roda em paralelo. Duas requisições tocando a
mesma linha ao mesmo tempo, um contador de limitação de taxa corrompido
por uma escrita concorrente: toda essa família de bugs simplesmente não
tem onde acontecer. A contrapartida aparece como um teto na taxa de
transferência máxima, mas para um servidor que atende uma comunidade em
vez de um serviço com milhões de usuários, esse laço único nunca vira
o gargalo.

## Do socket ao tópico do fórum

Uma mensagem recebida atravessa estas camadas, nesta ordem:

```
socket TCP
  → buffer de bytes brutos (connection_socket)
  → remoção do prefixo de tamanho (extract_length_prefixed_message)
  → handshake Noise em andamento, ou descriptografia se já estabelecido
  → decodificação do cabeçalho da trama (frame_codec)
  → limitação de taxa (por endereço, depois por identidade)
  → roteamento por família de mensagem (request_router)
  → handler (account_handler, forum_handler, dm_handler, ...)
  → repositório (post_repository, dm_repository, ...)
  → SQLite
```

A primeira bifurcação — handshake ou trama de aplicação — é decidida em
`connection_processor.cpp`. Enquanto `channel.is_established()`
retornar falso, cada mensagem recebida avança o handshake Noise em vez
de ser tratada como uma requisição; uma vez estabelecido o canal, tudo
o que chega é descriptografado e lido como uma trama.

O roteamento é dividido em famílias (sessão, conteúdo, social, DM,
denúncia) em vez de um único `switch` gigante sobre todos os tipos de
mensagem: as regras de estilo do projeto limitam uma função a sessenta
linhas, e um switch cobrindo os vinte e tantos tipos de mensagem
existentes ultrapassaria isso facilmente. `route_message` tenta cada
família em ordem e para assim que uma delas reconhece o tipo.

Cada handler conhece apenas sua própria tarefa —
`handle_dm_send_request` não faz ideia de que a tabela `posts` existe.
O que eles compartilham é o `handler_context` (config, logger, conexão
com o banco, conexão do cliente) e os repositórios, que são o único
lugar do código que toca o SQLite diretamente. Um handler que
construísse sua própria consulta SQL seria um sinal de alerta a esta
altura.

## O canal criptografado

O transporte não é TLS — não há nenhum cliente web a satisfazer, e o
TLS arrasta consigo o X.509 e as autoridades certificadoras, duas
coisas para as quais este projeto não tem nenhum uso. Em vez disso:
Noise NK sobre libsodium. O cliente já conhece de antemão a chave
pública estática do servidor (fixada na primeira conexão, ou lida de
um arquivo de conexão compartilhado); o handshake simplesmente falha
se um servidor impostor tentar responder em seu lugar.

Uma vez concluído o handshake (a etapa que o protocolo Noise chama de
`Split`), cada direção da comunicação ganha sua própria chave e seu
próprio contador de nonce dentro de `noise_transport`. Uma mensagem do
cliente e uma mensagem do servidor, portanto, nunca podem compartilhar
o mesmo nonce — se compartilhassem, o ChaCha20-Poly1305 deixaria de ser
seguro. Tudo o que vem depois, protocolo de aplicação incluído, existe
em texto claro somente nas duas máquinas das pontas.

## Um cliente, vários servidores

O cliente se parece mais com o Discord do que com o Slack: uma única
janela, uma barra de servidores na lateral, e cada servidor mantendo
sua própria identidade. Uma identidade compartilhada entre dois
servidores seria um identificador que dois administradores poderiam
cruzar para estabelecer que se trata da mesma pessoa nos dois lados —
algo que o projeto evita dando a cada servidor seu próprio par de
chaves, sem nenhum vínculo visível entre elas.

Duas estruturas dividem o estado:

- `app_state` contém tudo o que pertence à aplicação como um todo: o
  idioma, o slot ativo, a senha mantida em memória durante a sessão
  (nunca gravada em disco).
- `server_slot` contém tudo o que pertence a *um* servidor: sua
  conexão, sua identidade, sua sessão, e o estado de visualização do
  que estiver sendo exibido.

Os slots vivem em um `std::vector<std::unique_ptr<server_slot>>`,
nunca por valor. A razão se resume a um detalhe de implementação fácil
de quebrar por acidente: `client_session` guarda referências para
`server_connection` e para a identidade do slot. Se o vetor guardasse
`server_slot` por valor, um `push_back` que disparasse uma realocação
invalidaria essas referências sem avisar ninguém — o `unique_ptr` fixa
o endereço do slot de vez, então adicionar um servidor nunca move os
que já existiam.

## Onde procurar o quê

| Pergunta | Diretório |
|---|---|
| Como uma mensagem é estruturada no fio | `common/protocol/` |
| Criptografia, derivação de chaves, DMs | `common/crypto/` |
| Laço de rede, limitação de taxa, sessões | `server/net/` |
| Acesso ao SQLite | `server/db/` |
| Lógica de negócio por tipo de mensagem | `server/handlers/` |
| Comandos de administração (socket local) | `server/admin/` |
| Conexão, keystore, multi-servidor | `client/net/`, `client/keystore/` |
| Interface ImGui | `client/ui/` |
