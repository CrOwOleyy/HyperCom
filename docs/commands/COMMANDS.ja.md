# コマンド

[English](../../COMMANDS.md) · [Français](COMMANDS.fr.md) · [中文](COMMANDS.zh.md) · [हिन्दी](COMMANDS.hi.md) · [Español](COMMANDS.es.md) · [العربية](COMMANDS.ar.md) · [বাংলা](COMMANDS.bn.md) · [Português](COMMANDS.pt.md) · [Русский](COMMANDS.ru.md) · **日本語**

## ローカル設定

`env.ps1`(Windows)と `env.sh`(Linux/WSL)はバージョン管理されて
いない――自分のローカルのパスフレーズと自分のサーバーの公開鍵が
入っているからだ。初回の使い方:

```
cp env.example.ps1 env.ps1   # または env.example.sh -> env.sh
# その後、env.ps1 / env.sh を自分の値に編集する
```

## サーバーの管理

サーバーはローカルソケット(デフォルトでは
`run/hypercom-admin.sock`)を開き、`hypercom_adminctl` がそこに
コマンドを送る。C++ を書く必要も、SQL を打つ必要もない。

```
hypercom_adminctl help
hypercom_adminctl stats
hypercom_adminctl sessions
hypercom_adminctl sessions close 12
hypercom_adminctl motd set "土曜14時メンテナンス"
hypercom_adminctl motd clear
hypercom_adminctl backup backups/hypercom.db
```

ソケットがデフォルトのパスにない場合:

```
hypercom_adminctl --socket /var/run/hypercom-admin.sock stats
```

詳細とセキュリティ上の保証:docs/ADMIN.md §7。

## テストの実行

変更のたびに、再ビルドしてからスイートを実行する:

```
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

9 つのスイート:

| スイート | カバーする内容 |
|---|---|
| `protocol_parsing_test` | リーダーの境界、上限、UTF-8 検証、フレーミング |
| `crypto_round_trip_test` | ハンドシェイク、暗号化された DM、キーストア |
| `noise_official_vectors_test` | 公式 Noise ベクトルとのバイト単位の比較 |
| `message_roundtrip_session_test` | セッションメッセージ:hello、auth、ping、MOTD、status |
| `message_roundtrip_content_test` | フォーラム、投稿、コメント、スレッド |
| `message_roundtrip_social_test` | アカウント、prekey、プロフィール、フレンド、top 8、DM |
| `multi_server_identity_test` | サーバーごとのアイデンティティ、サーバーバー、スロットのライフサイクル |
| `fuzz_corpus_replay_test` | libFuzzer なしでファジングコーパスを再生 |
| `reconnection_test` | 同一オブジェクト上での完全な再ハンドシェイク(POSIX のみ) |

サニタイザー付き:

```
cmake -S . -B build-asan -DHYPERCOM_SANITIZER=address,undefined
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```

TSAN は別ディレクトリで実行する(ASAN とは併用不可)。WSL では
ASLR を無効化しないと起動を拒否される:

```
setarch -R ctest --test-dir build-tsan --output-on-failure
```

単体のテストを実行して詳細を見る:

```
build\windows\bin\RelWithDebInfo\noise_official_vectors_test.exe
```

## Windows で(PowerShell)

### 1. 依存関係のインストール
```
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\scripts\fetch_third_party.ps1
```

### 2. ビルドの設定

```
cmake -S . -B build/windows
```

### 3. プロジェクトのビルド
```
cmake --build build/windows --config RelWithDebInfo -j
```

### 4. プロジェクトの実行
#### 環境の読み込み
```
. .\env.ps1
```

#### ターミナル1でローカルにサーバーを起動
```
hserver
```

#### グラフィカルクライアント(GUI)を起動
```
hgui
```

#### 2つ目のクライアントを起動(2つ目のアカウントでテストする場合)
```
hgui --identity account2.key
```

## ウェルカムシーケンス

これは**アカウント作成時にだけ**再生され、それ以降の接続では
再生されない。使い捨てのアカウントを作らずにもう一度見るには:

```
hgui --replay-intro
```

あるいは新しいアイデンティティから始める――注意点として、その
ファイルがすでに存在していてはいけない。存在していればアカウントは
すでに登録済みとみなされ、イントロは発動しない:

```
hgui --identity new_account.key
```

`menu.mp3` の実際の長さ(約12.5秒)に合わせたタイムライン:

| タイミング | 何が起きるか |
|---|---|
| 0 → 6秒 | 中央に配置されたガラスのカード、「HyperCom の空間へようこそ。」 |
| 6秒 → 終了 | 3つの列が泡のように次々と立ち上がる |
| その後 | 通常のインターフェース、アニメーションはもうない |

`menu.mp3` を差し替えると、アニメーションは自動で再調整される――
長さはファイルから読み取られる。CMake はビルドのたびにそれを
実行ファイルの隣にコピーする。

音がない場合?それが妨げになることは決してない――イントロは時計に
沿って同じように進む。クライアントは起動時にその理由を表示する:

```
intro: music, duration used 12.5268 s
```

## Linux / WSL で

### 1. 依存関係のインストール
```
./scripts/fetch_third_party.sh
```

### 2. ビルドの設定
```
cmake -S . -B build/linux
```

### 3. プロジェクトのビルド
```
cmake --build build/linux -j
```

### 4. プロジェクトの実行
*(`hgui`、`hcli`、`hserver` のショートカットを読み込む)*

#### ローカルでサーバーを起動(ターミナル1)
```
hserver
```

#### 環境の読み込み(ターミナル2)
```
source env.sh
```

#### グラフィカルクライアント(GUI)を起動(ターミナル2)
```
hgui
```
