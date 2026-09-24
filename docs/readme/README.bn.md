# HyperCom

[English](../../README.md) · [Français](README.fr.md) · [中文](README.zh.md) · [हिन्दी](README.hi.md) · [Español](README.es.md) · [العربية](README.ar.md) · **বাংলা** · [Português](README.pt.md) · [Русский](README.ru.md) · [日本語](README.ja.md)

একটি বিকেন্দ্রীভূত, এন্ড-টু-এন্ড এনক্রিপ্টেড সোশ্যাল নেটওয়ার্ক:
Reddit-এর মতো কমিউনিটি ফোরাম, MySpace-এর মতো প্রোফাইল, এবং এমন
ব্যক্তিগত বার্তা যা কোনো সার্ভার পড়তে পারে না। C++20-এ একদম শুরু
থেকে লেখা, কোনো TLS বা ওয়েব নির্ভরতা ছাড়াই — libsodium-এর উপর হাতে
লেখা একটি Noise প্রোটোকল।

প্রতিটি কমিউনিটি নিজের সার্ভার নিজেই চালায়, Discord-এর মতো, কোনো
একক কেন্দ্রীয় সেবার বদলে। একটি মাস্টার পরিচয় দিয়ে যত খুশি সার্ভারে
যোগ দেওয়া যায়, আর প্রতিটি সার্ভারে থাকে আলাদা পরিচয়, যেগুলোর মধ্যে
কোনো সম্পর্ক খুঁজে বের করা যায় না।

সম্পূর্ণ ডকুমেন্টেশন: [ARCHITECTURE.md](../../ARCHITECTURE.md)
(অংশগুলো কীভাবে একসাথে কাজ করে), [docs/ADMIN.md](../ADMIN.md) (সার্ভার
পরিচালনা), [docs/THREAT_MODEL.md](../THREAT_MODEL.md) (কী সুরক্ষিত
আর কী নয়), [SECURITY.md](../../SECURITY.md) (একটা দুর্বলতা রিপোর্ট করা),
[COMMANDS.md](../../COMMANDS.md) (সম্পূর্ণ কমান্ড রেফারেন্স)।

## প্রয়োজনীয়তা

- CMake ≥ 3.20, একটি C++20 কম্পাইলার (Windows-এ MSVC, Linux-এ
  GCC/Clang)
- libsodium, SQLite এবং Dear ImGui: একটি স্ক্রিপ্টের মাধ্যমে একবারই
  আনা হয়, CMake কখনও নিজে থেকে আনে না

**সার্ভার শুধু Linux/WSL-এই বিল্ড হয়** (এটি epoll ও signalfd-এর
উপর নির্ভরশীল)। ক্লায়েন্ট — CLI এবং গ্রাফিক্যাল ইন্টারফেস দুটোই —
Windows এবং Linux উভয়েই বিল্ড হয়।

## সার্ভার পক্ষ (Linux / WSL)

```bash
./scripts/fetch_third_party.sh
cmake -S . -B build/linux
cmake --build build/linux -j
```

প্রথমবারের সেটআপ:

```bash
./build/linux/bin/hypercom_keygen server keys/server_static.key
chmod 600 keys/server_static.key
```

রিপোজিটরির রুটে `hypercom.conf` তৈরি করুন (সব অপশন
`docs/ADMIN.md §1-2`-এ ব্যাখ্যা করা আছে):

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

তারপর:

```bash
./build/linux/bin/hypercom_server hypercom.conf
```

চালু হওয়ার সময় সার্ভার তার পাবলিক কী দেখায় এবং
`run/hypercom-connect.txt` লেখে। এই ফাইলটি — বা শুধু তার ভেতরের
কী-টি — যে কাউকে সার্ভারে যোগ দিতে দেওয়া হয় বিশ্বস্ত কোনো মাধ্যম
দিয়ে, কখনোই সার্ভার থেকে নিজে সংগ্রহ করতে দেওয়া হয় না।

চলমান সার্ভার পরিচালনা (সেশন, MOTD, রিপোর্ট, ব্যান):
`docs/ADMIN.md §7`।

## ক্লায়েন্ট পক্ষ (Windows বা Linux)

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

বিল্ড হওয়ার পর এনভায়রনমেন্ট লোড করুন — এতে `hserver`, `hgui` এবং
`hcli` শর্টকাটগুলো পাওয়া যায়:

```powershell
cp env.example.ps1 env.ps1   # শুধু একবার, তারপর নিজের মান দিয়ে সম্পাদনা করুন
. .\env.ps1
```

পাওয়া আমন্ত্রণ লিঙ্ক দিয়ে একটি সার্ভারে যোগ দিন
(`hypercom://host:port#key`):

```
hcli server-add hypercom://203.0.113.7:7717#447a6def... my-server
hcli --server my-server whoami
```

অথবা সরাসরি অ্যাডমিনের দেওয়া কানেক্ট ফাইল দিয়ে:

```
hcli --connect-file hypercom-connect.txt whoami
```

গ্রাফিক্যাল ইন্টারফেস চালু করুন:

```
hgui
```

স্থানীয়ভাবে দুটি পরিচয় পাশাপাশি পরীক্ষা করার জন্য দ্বিতীয় একটি
অ্যাকাউন্ট:

```
hgui --identity account2.key
```

## পরীক্ষা

```bash
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

টেস্ট স্যুইটের বিস্তারিত, sanitizer দিয়ে বিল্ড, এবং ক্লায়েন্টের
স্বাগত সিকোয়েন্স: দেখুন [COMMANDS.md](../../COMMANDS.md)।
