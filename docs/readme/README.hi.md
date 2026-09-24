# HyperCom

[English](../../README.md) · [Français](README.fr.md) · [中文](README.zh.md) · **हिन्दी** · [Español](README.es.md) · [العربية](README.ar.md) · [বাংলা](README.bn.md) · [Português](README.pt.md) · [Русский](README.ru.md) · [日本語](README.ja.md)

एक विकेंद्रीकृत, एंड-टू-एंड एन्क्रिप्टेड सोशल नेटवर्क: Reddit जैसे
सामुदायिक फ़ोरम, MySpace जैसी प्रोफ़ाइल, और ऐसे निजी संदेश जिन्हें
कोई भी सर्वर पढ़ नहीं सकता। पूरी तरह से C++20 में शुरू से लिखा गया,
बिना TLS और बिना किसी वेब डिपेंडेंसी के — libsodium के ऊपर हाथ से
लिखा गया Noise प्रोटोकॉल।

हर समुदाय अपना खुद का सर्वर चलाता है, Discord की तरह, न कि किसी एक
केंद्रीय सेवा पर निर्भर होकर। एक ही मास्टर पहचान से जितने चाहें उतने
सर्वरों से जुड़ा जा सकता है, और हर सर्वर पर पहचान अलग होती है, जिन्हें
आपस में जोड़ा नहीं जा सकता।

पूरा दस्तावेज़: [ARCHITECTURE.md](../../ARCHITECTURE.md) (सारे हिस्से
कैसे जुड़ते हैं), [BRIEF.md](../../BRIEF.md) (डिज़ाइन के फ़ैसले),
[docs/ADMIN.md](../ADMIN.md) (सर्वर चलाना),
[docs/THREAT_MODEL.md](../THREAT_MODEL.md) (क्या सुरक्षित है और क्या
नहीं), [COMMANDS.md](../../COMMANDS.md) (सभी कमांड की पूरी सूची)।

## ज़रूरी चीज़ें

- CMake ≥ 3.20, एक C++20 कंपाइलर (Windows पर MSVC, Linux पर GCC/Clang)
- libsodium, SQLite और Dear ImGui: एक स्क्रिप्ट से एक बार में लाए
  जाते हैं, CMake कभी अपने आप नहीं लाता (देखें BRIEF.md 15)

**सर्वर सिर्फ़ Linux/WSL पर ही बिल्ड होता है** (यह epoll और signalfd
पर निर्भर है)। क्लाइंट — CLI और ग्राफ़िकल इंटरफ़ेस दोनों — Windows
और Linux दोनों पर बिल्ड होता है।

## सर्वर की तरफ़ (Linux / WSL)

```bash
./scripts/fetch_third_party.sh
cmake -S . -B build/linux
cmake --build build/linux -j
```

पहली बार सेटअप:

```bash
./build/linux/bin/hypercom_keygen server keys/server_static.key
chmod 600 keys/server_static.key
```

रिपॉज़िटरी की जड़ में `hypercom.conf` बनाएं (सारे विकल्प
`docs/ADMIN.md §1-2` में समझाए गए हैं):

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

फिर:

```bash
./build/linux/bin/hypercom_server hypercom.conf
```

शुरू होते ही सर्वर अपनी पब्लिक की दिखाता है और
`run/hypercom-connect.txt` लिखता है। यही फ़ाइल — या बस उसके अंदर की
की — उस व्यक्ति को दी जाती है जो सर्वर से जुड़ना चाहता है, किसी भरोसेमंद
ज़रिए से, और कभी भी सर्वर से खुद ही मंगवाकर नहीं।

चल रहे सर्वर का प्रबंधन (सेशन, MOTD, रिपोर्ट, बैन):
`docs/ADMIN.md §7`।

## क्लाइंट की तरफ़ (Windows या Linux)

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

बिल्ड होने के बाद एनवायरनमेंट लोड करें — इससे `hserver`, `hgui` और
`hcli` शॉर्टकट मिल जाते हैं:

```powershell
cp env.example.ps1 env.ps1   # एक ही बार, फिर अपनी वैल्यू के साथ बदलें
. .\env.ps1
```

मिले हुए इनवाइट लिंक से किसी सर्वर से जुड़ें
(`hypercom://host:port#key`):

```
hcli server-add hypercom://203.0.113.7:7717#447a6def... my-server
hcli --server my-server whoami
```

या एडमिन द्वारा दी गई कनेक्ट फ़ाइल का सीधे इस्तेमाल करें:

```
hcli --connect-file hypercom-connect.txt whoami
```

ग्राफ़िकल इंटरफ़ेस चलाएं:

```
hgui
```

लोकली दो पहचानों को साथ-साथ टेस्ट करने के लिए दूसरा खाता:

```
hgui --identity account2.key
```

## टेस्टिंग

```bash
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

टेस्ट सुइट्स का विवरण, sanitizer के साथ बिल्ड, और क्लाइंट की वेलकम
सीक्वेंस: देखें [COMMANDS.md](../../COMMANDS.md)।
