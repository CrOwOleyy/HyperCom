# HyperCom

[English](../../README.md) · [Français](README.fr.md) · [中文](README.zh.md) · [हिन्दी](README.hi.md) · [Español](README.es.md) · [العربية](README.ar.md) · [বাংলা](README.bn.md) · [Português](README.pt.md) · [Русский](README.ru.md) · **日本語**

サーバーが誰にも読めないエンドツーエンド暗号化の分散型 SNS。Reddit
風のコミュニティフォーラム、MySpace 風のプロフィール、そして誰にも
読まれないダイレクトメッセージ。C++20 でゼロから書かれており、TLS も
Web 系の依存関係も一切使わない——libsodium の上に自前で実装した
Noise プロトコルだけで成り立っている。

各コミュニティは中央サービスに頼らず、Discord のように自分専用の
サーバーを運用する。ひとつのマスターアイデンティティさえあれば
いくつでもサーバーに参加でき、それぞれのサーバー上のアイデンティティ
は互いに関連付けられない、別々のものになる。

詳しいドキュメント:[ARCHITECTURE.md](../../ARCHITECTURE.md)
(各部分がどう組み合わさっているか)、[docs/ADMIN.md](../ADMIN.md)
(サーバー運用)、[docs/THREAT_MODEL.md](../THREAT_MODEL.md)
(何が守られていて何が守られていないか)、
[SECURITY.md](../../SECURITY.md)(脆弱性の報告)、
[COMMANDS.md](../../COMMANDS.md)(コマンド一覧)。

## 必要なもの

- CMake 3.20 以上、C++20 対応コンパイラ(Windows なら MSVC、Linux
  なら GCC か Clang)
- libsodium、SQLite、Dear ImGui:スクリプトで一度だけ取得する。
  CMake が自動でダウンロードすることはない

**サーバーは Linux/WSL でしかビルドできない**(epoll と signalfd に
依存しているため)。クライアント側——CLI とグラフィカルインター
フェースの両方——は Windows でも Linux でもビルドできる。

## サーバー側(Linux / WSL)

```bash
./scripts/fetch_third_party.sh
cmake -S . -B build/linux
cmake --build build/linux -j
```

初回セットアップ:

```bash
./build/linux/bin/hypercom_keygen server keys/server_static.key
chmod 600 keys/server_static.key
```

リポジトリのルートに `hypercom.conf` を作成する(全オプションは
`docs/ADMIN.md §1-2` に説明あり):

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

そして:

```bash
./build/linux/bin/hypercom_server hypercom.conf
```

起動時、サーバーは自分の公開鍵を表示し、
`run/hypercom-connect.txt` を書き出す。このファイル(あるいは中身の
鍵だけ)を、参加してもらいたい相手に信頼できる経路で渡す。サーバー
自身から取得させることは絶対にしない。

稼働中のサーバーの管理(セッション、MOTD、通報、BAN):
`docs/ADMIN.md §7` 参照。

## クライアント側(Windows または Linux)

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

ビルドが終わったら環境を読み込む——`hserver`、`hgui`、`hcli` という
ショートカットが使えるようになる:

```powershell
cp env.example.ps1 env.ps1   # 最初に一度だけ、自分の値に編集する
. .\env.ps1
```

受け取った招待リンクでサーバーに参加する
(`hypercom://ホスト:ポート#鍵`):

```
hcli server-add hypercom://203.0.113.7:7717#447a6def... my-server
hcli --server my-server whoami
```

あるいは管理者から渡された接続ファイルを直接使う:

```
hcli --connect-file hypercom-connect.txt whoami
```

グラフィカルクライアントを起動する:

```
hgui
```

ローカルで二つのアイデンティティを並べて試すための二つ目のアカウント:

```
hgui --identity account2.key
```

## テスト

```bash
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

テストスイートの詳細、サニタイザー付きビルド、クライアントの起動
演出については [COMMANDS.md](../../COMMANDS.md) を参照。

## 現在の状態

これはプロトタイプだ――バグがあることは織り込み済みで、まだ荒削り
な部分も残っている。何か見つけたら issue を立てるか、すでに修正が
あるなら pull request を送ってほしい。それ以外の用件は
[leyy@pepepak.fr](mailto:leyy@pepepak.fr) まで。
