# HyperCom

[English](../../README.md) · [Français](README.fr.md) · [中文](README.zh.md) · [हिन्दी](README.hi.md) · [Español](README.es.md) · **العربية** · [বাংলা](README.bn.md) · [Português](README.pt.md) · [Русский](README.ru.md) · [日本語](README.ja.md)

شبكة اجتماعية لا مركزية ومشفّرة من طرف إلى طرف: منتديات مجتمعية على
طريقة Reddit، وملفات شخصية على طريقة MySpace، ورسائل خاصة لا يستطيع
أي خادم قراءتها. مكتوبة من الصفر بلغة C++20، دون استخدام TLS أو أي
اعتماديات ويب — بروتوكول Noise مكتوب يدويًا فوق مكتبة libsodium.

كل مجتمع يستضيف خادمه الخاص، على طريقة Discord، بدلاً من خدمة مركزية
واحدة. هوية رئيسية واحدة تتيح الانضمام إلى أي عدد من الخوادم، ولكل
خادم هوية منفصلة لا يمكن الربط بينها وبين الهويات الأخرى.

الوثائق الكاملة: [ARCHITECTURE.md](../../ARCHITECTURE.md) (كيفية ترابط
الأجزاء)، [docs/ADMIN.md](../ADMIN.md) (تشغيل الخادم)،
[docs/THREAT_MODEL.md](../THREAT_MODEL.md) (ما هو محمي وما هو غير
محمي)، [SECURITY.md](../../SECURITY.md) (الإبلاغ عن ثغرة أمنية)،
[COMMANDS.md](../../COMMANDS.md) (مرجع الأوامر الكامل).

## المتطلبات

- CMake بإصدار 3.20 أو أحدث، ومترجم يدعم C++20 (MSVC على Windows،
  وGCC أو Clang على Linux)
- مكتبات libsodium وSQLite وDear ImGui: تُجلب مرة واحدة عبر سكربت،
  ولا يجلبها CMake تلقائيًا أبدًا

**الخادم لا يُبنى إلا على Linux/WSL** (لأنه يعتمد على epoll
وsignalfd). أما العميل — سطر الأوامر والواجهة الرسومية — فيُبنى على
Windows وLinux معًا.

## جانب الخادم (Linux / WSL)

```bash
./scripts/fetch_third_party.sh
cmake -S . -B build/linux
cmake --build build/linux -j
```

الإعداد الأولي:

```bash
./build/linux/bin/hypercom_keygen server keys/server_static.key
chmod 600 keys/server_static.key
```

إنشاء ملف `hypercom.conf` في جذر المستودع (كل الخيارات موثّقة في
`docs/ADMIN.md §1-2`):

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

ثم:

```bash
./build/linux/bin/hypercom_server hypercom.conf
```

عند بدء التشغيل، يعرض الخادم مفتاحه العام ويكتب ملف
`run/hypercom-connect.txt`. هذا الملف — أو المفتاح الموجود فيه فقط —
هو ما يُسلَّم لأي شخص ينضم إلى الخادم، عبر قناة موثوقة، ولا يُترك له
الحصول عليه من الخادم نفسه أبدًا.

إدارة خادم قيد التشغيل (الجلسات، رسالة اليوم، البلاغات، الحظر):
`docs/ADMIN.md §7`.

## جانب العميل (Windows أو Linux)

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

تحميل البيئة بعد البناء — يوفّر هذا الاختصارات `hserver` و`hgui`
و`hcli`:

```powershell
cp env.example.ps1 env.ps1   # مرة واحدة فقط، ثم عدّله بقيمك الخاصة
. .\env.ps1
```

الانضمام إلى خادم باستخدام رابط الدعوة المستلَم
(`hypercom://المضيف:المنفذ#المفتاح`):

```
hcli server-add hypercom://203.0.113.7:7717#447a6def... my-server
hcli --server my-server whoami
```

أو مباشرة باستخدام ملف الاتصال الذي سلّمه المسؤول:

```
hcli --connect-file hypercom-connect.txt whoami
```

تشغيل الواجهة الرسومية:

```
hgui
```

حساب ثانٍ، لاختبار هويتين جنبًا إلى جنب محليًا:

```
hgui --identity account2.key
```

## الاختبار

```bash
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

تفاصيل مجموعات الاختبار، والبناء باستخدام أدوات sanitizer، وتسلسل
الترحيب في العميل: انظر [COMMANDS.md](../../COMMANDS.md).

## الحالة

هذا نموذج أولي: تُوقَّع الأخطاء، وبعض الأجزاء ما زالت خشنة إلى حد ما.
وجدت خطأ؟ افتح issue، أو أرسل pull request إذا كان لديك إصلاح جاهز.
لأي شيء آخر، راسل [leyy@pepepak.fr](mailto:leyy@pepepak.fr).
