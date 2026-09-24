# HyperCom

[English](../../README.md) · [Français](README.fr.md) · **中文** · [हिन्दी](README.hi.md) · [Español](README.es.md) · [العربية](README.ar.md) · [বাংলা](README.bn.md) · [Português](README.pt.md) · [Русский](README.ru.md) · [日本語](README.ja.md)

去中心化、端到端加密的社交网络:类似 Reddit 的社区论坛,类似 MySpace
的个人主页,任何服务器都无法读取的私信。用 C++20 从零编写,不依赖
TLS 或任何 Web 技术——在 libsodium 之上手写实现的 Noise 协议。

每个社区运行自己的服务器,就像 Discord 那样,而不是依赖一个中心化
服务。一个主身份可以加入任意数量的服务器,每个服务器上的身份互相
独立,彼此之间无法关联。

完整文档:[ARCHITECTURE.md](../../ARCHITECTURE.md)(各部分如何协同
工作)、[BRIEF.md](../../BRIEF.md)(设计决策)、
[docs/ADMIN.md](../ADMIN.md)(服务器运维)、
[docs/THREAT_MODEL.md](../THREAT_MODEL.md)(哪些受到保护,哪些没有)、
[COMMANDS.md](../../COMMANDS.md)(完整命令参考)。

## 依赖要求

- CMake ≥ 3.20,支持 C++20 的编译器(Windows 上用 MSVC,Linux 上用
  GCC/Clang)
- libsodium、SQLite 和 Dear ImGui:通过脚本一次性获取,CMake 从不会
  自动下载它们(见 BRIEF.md 15)

**服务器只能在 Linux/WSL 下编译**(依赖 epoll 和 signalfd)。客户端
——命令行和图形界面——在 Windows 和 Linux 上都能编译。

## 服务器端(Linux / WSL)

```bash
./scripts/fetch_third_party.sh
cmake -S . -B build/linux
cmake --build build/linux -j
```

首次配置:

```bash
./build/linux/bin/hypercom_keygen server keys/server_static.key
chmod 600 keys/server_static.key
```

在仓库根目录创建 `hypercom.conf`(所有选项都在
`docs/ADMIN.md §1-2` 中有说明):

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

然后:

```bash
./build/linux/bin/hypercom_server hypercom.conf
```

启动时,服务器会打印它的公钥,并写入
`run/hypercom-connect.txt`。这个文件——或者说文件里的那把公钥——就是
要交给想加入服务器的人的东西,通过一个可信的渠道传递,绝不能让对方
自己从服务器上获取。

管理正在运行的服务器(会话、MOTD、举报、封禁):见
`docs/ADMIN.md §7`。

## 客户端(Windows 或 Linux)

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

编译完成后加载环境变量——这会启用 `hserver`、`hgui` 和 `hcli`
这几个快捷命令:

```powershell
cp env.example.ps1 env.ps1   # 只需一次,之后编辑成你自己的值
. .\env.ps1
```

用收到的邀请链接加入服务器(`hypercom://主机:端口#密钥`):

```
hcli server-add hypercom://203.0.113.7:7717#447a6def... 我的服务器
hcli --server 我的服务器 whoami
```

或者直接使用管理员给的连接文件:

```
hcli --connect-file hypercom-connect.txt whoami
```

启动图形界面:

```
hgui
```

在本地并排测试两个身份,可以用第二个账号:

```
hgui --identity account2.key
```

## 测试

```bash
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

测试套件详情、sanitizer 构建方式以及客户端欢迎流程:见
[COMMANDS.md](../../COMMANDS.md)。
