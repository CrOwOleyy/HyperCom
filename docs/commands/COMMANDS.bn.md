# কমান্ড

[English](../../COMMANDS.md) · [Français](COMMANDS.fr.md) · [中文](COMMANDS.zh.md) · [हिन्दी](COMMANDS.hi.md) · [Español](COMMANDS.es.md) · [العربية](COMMANDS.ar.md) · **বাংলা** · [Português](COMMANDS.pt.md) · [Русский](COMMANDS.ru.md) · [日本語](COMMANDS.ja.md)

## স্থানীয় কনফিগারেশন

`env.ps1` (Windows) এবং `env.sh` (Linux/WSL) ভার্সন করা নেই: এগুলোতে
আপনার লোকাল পাসফ্রেজ এবং আপনার সার্ভারের পাবলিক কী থাকে। প্রথমবার
ব্যবহার:

```
cp env.example.ps1 env.ps1   # অথবা env.example.sh -> env.sh
# তারপর env.ps1 / env.sh নিজের মান দিয়ে এডিট করুন
```

## সার্ভার পরিচালনা

সার্ভার একটা লোকাল সকেট খোলে (ডিফল্টভাবে `run/hypercom-admin.sock`),
যেখানে `hypercom_adminctl` কমান্ড পাঠায়। কোনো C++ লেখার দরকার নেই,
কোনো SQL টাইপ করার দরকার নেই।

```
hypercom_adminctl help
hypercom_adminctl stats
hypercom_adminctl sessions
hypercom_adminctl sessions close 12
hypercom_adminctl motd set "শনিবার দুপুর ২টায় মেইনটেন্যান্স"
hypercom_adminctl motd clear
hypercom_adminctl backup backups/hypercom.db
```

সকেট যদি ডিফল্ট পাথে না থাকে:

```
hypercom_adminctl --socket /var/run/hypercom-admin.sock stats
```

বিস্তারিত এবং নিরাপত্তার নিশ্চয়তা: docs/ADMIN.md §7।

## টেস্ট চালানো

প্রতিটা পরিবর্তনের পর, আবার বিল্ড করে স্যুইট চালান:

```
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

নয়টা স্যুইট:

| স্যুইট | কী কভার করে |
|---|---|
| `protocol_parsing_test` | রিডারের সীমা, ক্যাপ, UTF-8 যাচাই, ফ্রেমিং |
| `crypto_round_trip_test` | হ্যান্ডশেক, এনক্রিপ্টেড DM, কীস্টোর |
| `noise_official_vectors_test` | একটা অফিশিয়াল Noise ভেক্টরের সাথে বাইট-বাই-বাইট তুলনা |
| `message_roundtrip_session_test` | সেশন মেসেজ: hello, auth, ping, MOTD, status |
| `message_roundtrip_content_test` | ফোরাম, পোস্ট, কমেন্ট, থ্রেড |
| `message_roundtrip_social_test` | অ্যাকাউন্ট, prekeys, প্রোফাইল, বন্ধু, top 8, DM |
| `multi_server_identity_test` | প্রতি সার্ভারের আলাদা পরিচয়, সার্ভার বার, স্লটের জীবনচক্র |
| `fuzz_corpus_replay_test` | libFuzzer ছাড়াই ফাজিং কর্পাস পুনরায় চালানো |
| `reconnection_test` | একই অবজেক্টে পূর্ণ পুনরায়-হ্যান্ডশেক (শুধু POSIX) |

sanitizer সহ:

```
cmake -S . -B build-asan -DHYPERCOM_SANITIZER=address,undefined
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```

TSAN আলাদা ডিরেক্টরি থেকে চলে (ASAN-এর সাথে সামঞ্জস্যপূর্ণ নয়)। WSL-এ
ASLR বন্ধ করতে হয়, নাহলে এটা শুরু হতে অস্বীকার করে:

```
setarch -R ctest --test-dir build-tsan --output-on-failure
```

শুধু একটা টেস্ট, বিস্তারিত দেখার জন্য:

```
build\windows\bin\RelWithDebInfo\noise_official_vectors_test.exe
```

## Windows-এ (PowerShell)

### ১. ডিপেন্ডেন্সি ইনস্টল করুন
```
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\scripts\fetch_third_party.ps1
```

### ২. বিল্ড কনফিগার করুন

```
cmake -S . -B build/windows
```

### ৩. প্রজেক্ট বিল্ড করুন
```
cmake --build build/windows --config RelWithDebInfo -j
```

### ৪. প্রজেক্ট চালান
#### এনভায়রনমেন্ট লোড করুন
```
. .\env.ps1
```

#### টার্মিনাল ১-এ লোকালি সার্ভার চালু করুন
```
hserver
```

#### গ্রাফিক্যাল ইন্টারফেস ক্লায়েন্ট (GUI) চালান
```
hgui
```

#### দ্বিতীয় ক্লায়েন্ট চালান (দ্বিতীয় অ্যাকাউন্ট দিয়ে টেস্ট করতে)
```
hgui --identity account2.key
```

## স্বাগত সিকোয়েন্স

এটা শুধু **অ্যাকাউন্ট তৈরির সময়** চলে, পরের কানেকশনে কখনো না।
কোনো ডিসপোজেবল অ্যাকাউন্ট তৈরি না করে আবার দেখতে চাইলে:

```
hgui --replay-intro
```

অথবা একদম নতুন পরিচয় থেকে শুরু করে — খেয়াল রাখুন, ফাইলটা আগে থেকে
থাকা যাবে না, নাহলে অ্যাকাউন্ট ইতিমধ্যে রেজিস্টার্ড ধরে নেওয়া হবে
এবং ইন্ট্রো শুরু হবে না:

```
hgui --identity new_account.key
```

`menu.mp3`-এর আসল দৈর্ঘ্যের (~১২.৫ সেকেন্ড) সাথে মেলানো সময়সূচি:

| মুহূর্ত | কী ঘটে |
|---|---|
| ০ → ৬ সেকেন্ড | মাঝখানে কাচের কার্ড, "HyperCom স্পেসে স্বাগতম।" |
| ৬ সেকেন্ড → শেষ | তিনটা কলাম একটার পর একটা বাবল আকারে উঠে আসে |
| তারপর | স্বাভাবিক ইন্টারফেস, আর কোনো অ্যানিমেশন নেই |

`menu.mp3` পরিবর্তন করলে অ্যানিমেশন নিজে থেকেই আবার ক্যালিব্রেট হয়:
দৈর্ঘ্য ফাইল থেকে পড়া হয়। প্রতিটা বিল্ডে CMake এটাকে এক্সিকিউটেবলের
পাশে কপি করে।

শব্দ নেই? এটা কখনো বাধা হয় না — ঘড়ি অনুযায়ী ইন্ট্রো একইভাবে চলে।
ক্লায়েন্ট শুরুতেই কারণ দেখায়:

```
intro: music, duration used 12.5268 s
```

## Linux / WSL-এ

### ১. ডিপেন্ডেন্সি ইনস্টল করুন
```
./scripts/fetch_third_party.sh
```

### ২. বিল্ড কনফিগার করুন
```
cmake -S . -B build/linux
```

### ৩. প্রজেক্ট বিল্ড করুন
```
cmake --build build/linux -j
```

### ৪. প্রজেক্ট চালান
*(`hgui`, `hcli` এবং `hserver` শর্টকাট লোড করে)*

#### লোকালি সার্ভার চালু করুন (টার্মিনাল ১)
```
hserver
```

#### এনভায়রনমেন্ট লোড করুন (টার্মিনাল ২)
```
source env.sh
```

#### গ্রাফিক্যাল ইন্টারফেস ক্লায়েন্ট (GUI) চালান (টার্মিনাল ২)
```
hgui
```
