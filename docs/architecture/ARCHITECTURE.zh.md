# Architecture

[English](../../ARCHITECTURE.md) · [Français](ARCHITECTURE.fr.md) · **中文** · [हिन्दी](ARCHITECTURE.hi.md) · [Español](ARCHITECTURE.es.md) · [العربية](ARCHITECTURE.ar.md) · [বাংলা](ARCHITECTURE.bn.md) · [Português](ARCHITECTURE.pt.md) · [Русский](ARCHITECTURE.ru.md) · [日本語](ARCHITECTURE.ja.md)

本文档解释各个部分是如何组合在一起的:从客户端连接的那一刻,到一条
消息最终出现在论坛帖子或私信收件箱里的那一刻,中间发生了什么。关于
某个具体主题的细节,可以参考其他文档:
[PROTOCOL.md](../PROTOCOL.md)讲的是帧格式,
[SCHEMA.md](../SCHEMA.md)讲的是数据库,
[THREAT_MODEL.md](../THREAT_MODEL.md)讲的是哪些受保护、哪些不受保护,
[ADMIN.md](../ADMIN.md)讲的是如何运维服务器。

## 服务器里没有任何东西是并行运行的

`server_runtime::run_until_stopped` 就是一个 `epoll` 循环,仅此而已。
没有为每个连接开一个线程,也没有线程池。每一个连接、每一次数据库
查询、每一次加密运算,都依次排队通过同一个循环处理,`server/`
目录下没有一把互斥锁——因为没有任何东西是并发执行的,自然也就没有
什么需要保护。两个请求同时改同一行数据、并发写入把限流计数器搞坏,
这整一类 bug 根本没有发生的机会。代价体现在最大吞吐量的上限上,但
对于一个服务某个社区、而不是服务几百万用户的服务器来说,这个单一
循环从来不会成为瓶颈。

## 从 socket 到论坛帖子

一条收到的消息会依次经过这些层:

```
TCP socket
  → 原始字节缓冲区(connection_socket)
  → 去掉长度前缀(extract_length_prefixed_message)
  → 正在进行的 Noise 握手,或者已建立连接后的解密
  → 解析帧头(frame_codec)
  → 限流(先按地址,再按身份)
  → 按消息类别路由(request_router)
  → 处理函数(account_handler、forum_handler、dm_handler……)
  → 仓储层(post_repository、dm_repository……)
  → SQLite
```

第一步分支——是握手还是应用层帧——在 `connection_processor.cpp`
里决定。只要 `channel.is_established()` 返回 false,每一条收到的
消息都会用来推进 Noise 握手,而不会被当作正式请求处理;一旦通道
建立完成,之后收到的一切都会被解密并当作一帧来读取。

路由被拆分成几个类别(会话、内容、社交、私信、举报),而不是用一个
巨大的 `switch` 涵盖所有消息类型:项目的代码规范把一个函数限制在
六十行以内,而一个要覆盖现有二十多种消息类型的 switch 语句会轻松
突破这个限制。`route_message` 依次尝试每个类别,一旦有类别识别出
该类型就立刻停止。

每个处理函数只管自己那一份工作——`handle_dm_send_request` 根本不
知道 `posts` 表的存在。它们共用的是 `handler_context`(配置、日志
记录器、数据库连接、客户端连接)以及仓储层,而仓储层是代码库里唯一
直接接触 SQLite 的地方。走到这一步,如果哪个处理函数自己拼 SQL
查询,那就是一个危险信号。

## 加密通道

传输层用的不是 TLS——这里没有需要照顾的网页客户端,而 TLS 会带来
X.509 和证书颁发机构这两样东西,这个项目根本用不上。取而代之的是:
基于 libsodium 实现的 Noise NK。客户端事先就知道服务器的静态公钥
(在首次连接时锁定,或者从共享的连接文件中读取);如果有冒充的
服务器试图代替真正的服务器应答,握手会直接失败。

握手完成之后(Noise 协议把这一步称为 `Split`),通信的每个方向都会
在 `noise_transport` 里拿到属于自己的密钥和自己的 nonce
计数器。这样一来,客户端发出的消息和服务器发出的消息就绝不可能
共用同一个 nonce——如果共用了,ChaCha20-Poly1305
的安全性就不再成立。此后的一切,包括应用层协议在内,只有在两端的
机器上才以明文形式存在。

## 一个客户端,多个服务器

这个客户端的设计思路更接近 Discord,而不是 Slack:一个窗口,侧边有
一条服务器栏,每个服务器各自保留自己的身份。如果两个服务器共用同一
个身份,就相当于给了两个管理员一个可以互相比对的标识符,从而确认
双方面对的是同一个人——项目正是通过给每个服务器分配独立的密钥对、
彼此之间没有任何可见关联,来避免这种情况。

状态被拆分到两个结构里:

- `app_state` 存放属于整个应用程序的东西:语言、当前激活的
  服务器槽位、只在内存里保留一整个会话期间、从不写入磁盘的密码。
- `server_slot` 存放属于*某一个*服务器的一切:它的连接、它的身份、
  它的会话,以及它当前显示内容的视图状态。

服务器槽位保存在 `std::vector<std::unique_ptr<server_slot>>` 里,
从来不是按值存储。原因归结为一个很容易被不小心破坏的实现细节:
`client_session` 保存着指向 `server_connection` 和该槽位身份的
引用。如果这个 vector 按值保存 `server_slot`,一次触发扩容的
`push_back` 就会在谁都不知情的情况下让这些引用失效——而
`unique_ptr` 把槽位的地址永久固定下来,所以添加新服务器永远不会
挪动已经存在的那些。

## 该去哪里找什么

| 问题 | 目录 |
|---|---|
| 消息在线上是如何构造的 | `common/protocol/` |
| 加密、密钥派生、私信 | `common/crypto/` |
| 网络循环、限流、会话 | `server/net/` |
| SQLite 访问 | `server/db/` |
| 按消息类型划分的业务逻辑 | `server/handlers/` |
| 管理命令(本地 socket) | `server/admin/` |
| 连接、密钥库、多服务器 | `client/net/`、`client/keystore/` |
| ImGui 界面 | `client/ui/` |
