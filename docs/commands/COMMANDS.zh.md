# 命令

[English](../../COMMANDS.md) · [Français](COMMANDS.fr.md) · **中文** · [हिन्दी](COMMANDS.hi.md) · [Español](COMMANDS.es.md) · [العربية](COMMANDS.ar.md) · [বাংলা](COMMANDS.bn.md) · [Português](COMMANDS.pt.md) · [Русский](COMMANDS.ru.md) · [日本語](COMMANDS.ja.md)

## 本地配置

`env.ps1`(Windows)和 `env.sh`(Linux/WSL)不纳入版本控制:它们
包含你本地的密码短语和你服务器的公钥。首次使用:

```
cp env.example.ps1 env.ps1   # 或 env.example.sh -> env.sh
# 然后用你自己的值编辑 env.ps1 / env.sh
```

## 管理服务器

服务器会打开一个本地 socket(默认是 `run/hypercom-admin.sock`),
`hypercom_adminctl` 通过它发送命令。不用写 C++,不用敲 SQL。

```
hypercom_adminctl help
hypercom_adminctl stats
hypercom_adminctl sessions
hypercom_adminctl sessions close 12
hypercom_adminctl motd set "周六 14 点维护"
hypercom_adminctl motd clear
hypercom_adminctl backup backups/hypercom.db
```

如果 socket 不在默认路径:

```
hypercom_adminctl --socket /var/run/hypercom-admin.sock stats
```

详情与安全保证:docs/ADMIN.md §7。

## 运行测试

每次修改后,先重新编译再跑测试套件:

```
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

九个测试套件:

| 套件 | 覆盖内容 |
|---|---|
| `protocol_parsing_test` | 读取器边界、上限、UTF-8 校验、分帧 |
| `crypto_round_trip_test` | 握手、加密私信、密钥库 |
| `noise_official_vectors_test` | 与官方 Noise 测试向量逐字节比对 |
| `message_roundtrip_session_test` | 会话消息:hello、auth、ping、MOTD、status |
| `message_roundtrip_content_test` | 论坛、帖子、评论、帖子串 |
| `message_roundtrip_social_test` | 账号、prekeys、个人资料、好友、top 8、私信 |
| `multi_server_identity_test` | 每服务器独立身份、服务器栏、槽位生命周期 |
| `fuzz_corpus_replay_test` | 重放模糊测试语料库,不依赖 libFuzzer |
| `reconnection_test` | 在同一对象上完整重新握手(仅 POSIX) |

在 sanitizer 下:

```
cmake -S . -B build-asan -DHYPERCOM_SANITIZER=address,undefined
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```

TSAN 需要在单独目录下运行(与 ASAN 不兼容)。在 WSL 下要先关闭
ASLR,否则它会拒绝启动:

```
setarch -R ctest --test-dir build-tsan --output-on-failure
```

单独运行一个测试,查看细节:

```
build\windows\bin\RelWithDebInfo\noise_official_vectors_test.exe
```

## 在 Windows 上(PowerShell)

### 1. 安装依赖
```
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\scripts\fetch_third_party.ps1
```

### 2. 配置构建

```
cmake -S . -B build/windows
```

### 3. 编译项目
```
cmake --build build/windows --config RelWithDebInfo -j
```

### 4. 运行项目
#### 加载环境变量
```
. .\env.ps1
```

#### 在终端 1 本地启动服务器
```
hserver
```

#### 启动图形界面客户端(GUI)
```
hgui
```

#### 启动第二个客户端(用第二个账号测试)
```
hgui --identity account2.key
```

## 欢迎序列

它只在**创建账号时**播放,之后再连接就不会出现。想在不创建一次性
账号的情况下重看它:

```
hgui --replay-intro
```

或者用一个全新的身份重新开始——注意,该文件不能已经存在,否则账号
已经注册,intro 就不会触发:

```
hgui --identity new_account.key
```

时间轴,按 `menu.mp3` 的真实时长(约 12.5 秒)校准:

| 时刻 | 发生的事 |
|---|---|
| 0 → 6 秒 | 居中的玻璃卡片,"欢迎来到 HyperCom 空间。" |
| 6 秒 → 结束 | 三列依次以气泡的形式升起 |
| 之后 | 正常界面,不再有动画 |

替换 `menu.mp3` 会自动重新校准动画:时长是从文件里读出来的。每次
构建,CMake 都会把它复制到可执行文件旁边。

没有声音?这从来都不会造成阻塞——intro 会按同样的时钟播放完。客户端
启动时会打印原因:

```
intro: music, duration used 12.5268 s
```

## 在 Linux / WSL 上

### 1. 安装依赖
```
./scripts/fetch_third_party.sh
```

### 2. 配置构建
```
cmake -S . -B build/linux
```

### 3. 编译项目
```
cmake --build build/linux -j
```

### 4. 运行项目
*(会加载 `hgui`、`hcli` 和 `hserver` 这几个快捷命令)*

#### 本地启动服务器(终端 1)
```
hserver
```

#### 加载环境变量(终端 2)
```
source env.sh
```

#### 启动图形界面客户端(GUI)(终端 2)
```
hgui
```
