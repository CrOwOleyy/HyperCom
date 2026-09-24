# कमांड

[English](../../COMMANDS.md) · [Français](COMMANDS.fr.md) · [中文](COMMANDS.zh.md) · **हिन्दी** · [Español](COMMANDS.es.md) · [العربية](COMMANDS.ar.md) · [বাংলা](COMMANDS.bn.md) · [Português](COMMANDS.pt.md) · [Русский](COMMANDS.ru.md) · [日本語](COMMANDS.ja.md)

## लोकल कॉन्फ़िगरेशन

`env.ps1` (Windows) और `env.sh` (Linux/WSL) वर्ज़न में नहीं हैं: इनमें
आपकी लोकल पासफ़्रेज़ और आपके सर्वर की पब्लिक की होती है। पहली बार
इस्तेमाल:

```
cp env.example.ps1 env.ps1   # या env.example.sh -> env.sh
# फिर env.ps1 / env.sh को अपनी वैल्यू से एडिट करें
```

## सर्वर प्रबंधित करना

सर्वर एक लोकल सॉकेट खोलता है (डिफ़ॉल्ट रूप से
`run/hypercom-admin.sock`), जिस पर `hypercom_adminctl` कमांड भेजता है।
न कोई C++ लिखना, न कोई SQL टाइप करना।

```
hypercom_adminctl help
hypercom_adminctl stats
hypercom_adminctl sessions
hypercom_adminctl sessions close 12
hypercom_adminctl motd set "शनिवार दोपहर 2 बजे मेंटेनेंस"
hypercom_adminctl motd clear
hypercom_adminctl backup backups/hypercom.db
```

अगर सॉकेट डिफ़ॉल्ट पाथ पर नहीं है:

```
hypercom_adminctl --socket /var/run/hypercom-admin.sock stats
```

विवरण और सुरक्षा गारंटी: docs/ADMIN.md §7।

## टेस्ट चलाना

हर बदलाव के बाद, फिर से बिल्ड करें और सुइट चलाएं:

```
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

नौ सुइट:

| सुइट | क्या कवर करता है |
|---|---|
| `protocol_parsing_test` | रीडर की सीमाएं, कैप, UTF-8 वैलिडेशन, फ़्रेमिंग |
| `crypto_round_trip_test` | हैंडशेक, एन्क्रिप्टेड DM, कीस्टोर |
| `noise_official_vectors_test` | एक आधिकारिक Noise वेक्टर से बाइट-दर-बाइट तुलना |
| `message_roundtrip_session_test` | सेशन मैसेज: hello, auth, ping, MOTD, status |
| `message_roundtrip_content_test` | फ़ोरम, पोस्ट, कमेंट, थ्रेड |
| `message_roundtrip_social_test` | अकाउंट, prekeys, प्रोफ़ाइल, दोस्त, top 8, DM |
| `multi_server_identity_test` | हर सर्वर की अलग पहचान, सर्वर बार, स्लॉट का जीवनचक्र |
| `fuzz_corpus_replay_test` | libFuzzer के बिना फ़ज़िंग कॉर्पस को दोबारा चलाना |
| `reconnection_test` | एक ही ऑब्जेक्ट पर पूरा दोबारा-हैंडशेक (सिर्फ़ POSIX) |

sanitizer के साथ:

```
cmake -S . -B build-asan -DHYPERCOM_SANITIZER=address,undefined
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```

TSAN अलग डायरेक्टरी से चलता है (ASAN के साथ इस्तेमाल नहीं हो सकता)।
WSL पर ASLR बंद करना ज़रूरी है, वरना यह शुरू होने से मना कर देता है:

```
setarch -R ctest --test-dir build-tsan --output-on-failure
```

एक अकेला टेस्ट, डीटेल देखने के लिए:

```
build\windows\bin\RelWithDebInfo\noise_official_vectors_test.exe
```

## Windows पर (PowerShell)

### 1. डिपेंडेंसी इंस्टॉल करें
```
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\scripts\fetch_third_party.ps1
```

### 2. बिल्ड कॉन्फ़िगर करें

```
cmake -S . -B build/windows
```

### 3. प्रोजेक्ट बिल्ड करें
```
cmake --build build/windows --config RelWithDebInfo -j
```

### 4. प्रोजेक्ट चलाएं
#### एनवायरनमेंट लोड करें
```
. .\env.ps1
```

#### टर्मिनल 1 में सर्वर लोकली शुरू करें
```
hserver
```

#### ग्राफ़िकल इंटरफ़ेस क्लाइंट (GUI) चलाएं
```
hgui
```

#### दूसरा क्लाइंट चलाएं (दूसरे अकाउंट से टेस्ट करने के लिए)
```
hgui --identity account2.key
```

## वेलकम सीक्वेंस

यह **सिर्फ़ अकाउंट बनाते समय** चलता है, बाद के कनेक्शन में कभी नहीं।
बिना कोई डिस्पोज़ेबल अकाउंट बनाए इसे दोबारा देखने के लिए:

```
hgui --replay-intro
```

या एक नई पहचान से शुरू करके — ध्यान रहे, फ़ाइल पहले से मौजूद नहीं
होनी चाहिए, वरना अकाउंट पहले से रजिस्टर्ड मान लिया जाता है और intro
शुरू नहीं होता:

```
hgui --identity new_account.key
```

`menu.mp3` की असली अवधि (~12.5 सेकंड) पर कैलिब्रेटेड क्रम:

| पल | क्या होता है |
|---|---|
| 0 → 6 सेकंड | बीच में शीशे जैसा कार्ड, "HyperCom के स्पेस में आपका स्वागत है।" |
| 6 सेकंड → अंत | तीनों कॉलम एक के बाद एक बबल की तरह ऊपर उठते हैं |
| उसके बाद | नॉर्मल इंटरफ़ेस, कोई और एनिमेशन नहीं |

`menu.mp3` बदलने से एनिमेशन अपने आप दोबारा कैलिब्रेट हो जाता है:
अवधि फ़ाइल से पढ़ी जाती है। हर बिल्ड पर CMake इसे एक्ज़िक्यूटेबल के
बगल में कॉपी करता है।

आवाज़ नहीं है? यह कभी रुकावट नहीं बनता — intro घड़ी के हिसाब से वैसा
ही चलता है। क्लाइंट शुरू होते ही वजह दिखाता है:

```
intro: music, duration used 12.5268 s
```

## Linux / WSL पर

### 1. डिपेंडेंसी इंस्टॉल करें
```
./scripts/fetch_third_party.sh
```

### 2. बिल्ड कॉन्फ़िगर करें
```
cmake -S . -B build/linux
```

### 3. प्रोजेक्ट बिल्ड करें
```
cmake --build build/linux -j
```

### 4. प्रोजेक्ट चलाएं
*(`hgui`, `hcli` और `hserver` शॉर्टकट लोड करता है)*

#### सर्वर लोकली शुरू करें (टर्मिनल 1)
```
hserver
```

#### एनवायरनमेंट लोड करें (टर्मिनल 2)
```
source env.sh
```

#### ग्राफ़िकल इंटरफ़ेस क्लाइंट (GUI) चलाएं (टर्मिनल 2)
```
hgui
```
