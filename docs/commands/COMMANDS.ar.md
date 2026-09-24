# الأوامر

[English](../../COMMANDS.md) · [Français](COMMANDS.fr.md) · [中文](COMMANDS.zh.md) · [हिन्दी](COMMANDS.hi.md) · [Español](COMMANDS.es.md) · **العربية** · [বাংলা](COMMANDS.bn.md) · [Português](COMMANDS.pt.md) · [Русский](COMMANDS.ru.md) · [日本語](COMMANDS.ja.md)

## الإعداد المحلي

لا يتم تتبّع `env.ps1` (على Windows) و`env.sh` (على Linux/WSL) في
النسخ: فهما يحتويان على عبارة مرورك المحلية والمفتاح العام لخادمك.
الاستخدام الأول:

```
cp env.example.ps1 env.ps1   # أو env.example.sh -> env.sh
# ثم تحرير env.ps1 / env.sh بقيمك الخاصة
```

## إدارة الخادم

يفتح الخادم مقبسًا محليًا (`run/hypercom-admin.sock` افتراضيًا) يرسل
إليه `hypercom_adminctl` الأوامر. لا حاجة لكتابة C++، ولا لكتابة SQL.

```
hypercom_adminctl help
hypercom_adminctl stats
hypercom_adminctl sessions
hypercom_adminctl sessions close 12
hypercom_adminctl motd set "صيانة يوم السبت الساعة 2 ظهرًا"
hypercom_adminctl motd clear
hypercom_adminctl backup backups/hypercom.db
```

إذا لم يكن المقبس في المسار الافتراضي:

```
hypercom_adminctl --socket /var/run/hypercom-admin.sock stats
```

التفاصيل وضمانات الأمان: docs/ADMIN.md §7.

## تشغيل الاختبارات

بعد كل تعديل، أعد البناء ثم شغّل المجموعة:

```
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

تسع مجموعات:

| المجموعة | ما تغطيه |
|---|---|
| `protocol_parsing_test` | حدود القارئ، السقوف، التحقق من UTF-8، التأطير |
| `crypto_round_trip_test` | المصافحة، الرسائل الخاصة المشفرة، مخزن المفاتيح |
| `noise_official_vectors_test` | مقارنة بايت ببايت مع متجه Noise رسمي |
| `message_roundtrip_session_test` | رسائل الجلسة: hello، auth، ping، MOTD، status |
| `message_roundtrip_content_test` | المنتديات، المنشورات، التعليقات، المواضيع |
| `message_roundtrip_social_test` | الحساب، المفاتيح المسبقة، الملفات الشخصية، الأصدقاء، top 8، الرسائل الخاصة |
| `multi_server_identity_test` | الهويات لكل خادم، شريط الخوادم، دورة حياة الفتحات |
| `fuzz_corpus_replay_test` | إعادة تشغيل مجموعة الفَزّ، دون libFuzzer |
| `reconnection_test` | مصافحة كاملة جديدة على نفس الكائن (POSIX فقط) |

مع أدوات sanitizer:

```
cmake -S . -B build-asan -DHYPERCOM_SANITIZER=address,undefined
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```

يعمل TSAN من مجلد منفصل (غير متوافق مع ASAN). على WSL، يجب تعطيل
ASLR، وإلا فإنه يرفض البدء:

```
setarch -R ctest --test-dir build-tsan --output-on-failure
```

اختبار واحد فقط، لرؤية التفاصيل:

```
build\windows\bin\RelWithDebInfo\noise_official_vectors_test.exe
```

## على Windows (PowerShell)

### 1. تثبيت الاعتماديات
```
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\scripts\fetch_third_party.ps1
```

### 2. إعداد البناء

```
cmake -S . -B build/windows
```

### 3. بناء المشروع
```
cmake --build build/windows --config RelWithDebInfo -j
```

### 4. تشغيل المشروع
#### تحميل البيئة
```
. .\env.ps1
```

#### تشغيل الخادم محليًا في الطرفية 1
```
hserver
```

#### تشغيل العميل بالواجهة الرسومية (GUI)
```
hgui
```

#### تشغيل عميل ثانٍ (للاختبار بحساب ثانٍ)
```
hgui --identity account2.key
```

## تسلسل الترحيب

يُعرض فقط **عند إنشاء حساب**، ولا يُعرض أبدًا في الاتصالات اللاحقة.
لإعادة مشاهدته دون إنشاء حساب مؤقت:

```
hgui --replay-intro
```

أو بالانطلاق من هوية جديدة — تنبيه: يجب ألا يكون الملف موجودًا
مسبقًا، وإلا فالحساب مسجَّل بالفعل ولن تنطلق المقدمة:

```
hgui --identity new_account.key
```

التسلسل، مضبوط على المدة الفعلية لملف `menu.mp3` (~12.5 ثانية):

| اللحظة | ما يحدث |
|---|---|
| 0 → 6 ثوانٍ | بطاقة زجاجية في المنتصف، "أهلًا بك في فضاء HyperCom." |
| 6 ثوانٍ → النهاية | الأعمدة الثلاثة ترتفع الواحد تلو الآخر، كفقاعات |
| بعد ذلك | واجهة عادية، لا مزيد من الحركة |

استبدال `menu.mp3` يعيد ضبط الحركة تلقائيًا: تُقرأ المدة من الملف.
ينسخه CMake بجانب الملف التنفيذي في كل بناء.

لا صوت؟ لا يُعطّل هذا أبدًا — تُعرض المقدمة بنفس الطريقة اعتمادًا على
الساعة. يعرض العميل السبب عند البدء:

```
intro: music, duration used 12.5268 s
```

## على Linux / WSL

### 1. تثبيت الاعتماديات
```
./scripts/fetch_third_party.sh
```

### 2. إعداد البناء
```
cmake -S . -B build/linux
```

### 3. بناء المشروع
```
cmake --build build/linux -j
```

### 4. تشغيل المشروع
*(يحمّل اختصارات `hgui` و`hcli` و`hserver`)*

#### تشغيل الخادم محليًا (الطرفية 1)
```
hserver
```

#### تحميل البيئة (الطرفية 2)
```
source env.sh
```

#### تشغيل العميل بالواجهة الرسومية (GUI) (الطرفية 2)
```
hgui
```
