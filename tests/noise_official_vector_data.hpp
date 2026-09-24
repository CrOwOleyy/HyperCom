#pragma once

#include <array>
#include <string_view>

namespace hypercom::tests::noise_vector {

// Official Noise_NK_25519_ChaChaPoly_SHA256 vector, with no PSK or fallback.
//
// Source: noise-c (https://github.com/rweather/noise-c),
// tests/vector/noise-c-basic.txt -- the only vector in that file that
// matches our suite exactly (no PSK, no fallback). Source copy kept at
// tests/vectors/noise_nk_25519_chachapoly_sha256.json for auditing.
// Generated file, do not edit by hand -- regenerate from the JSON instead.

constexpr std::string_view PROTOCOL_NAME = "Noise_NK_25519_ChaChaPoly_SHA256";

constexpr std::string_view PROLOGUE_HEX = // 11 bytes
    "50726f6c6f677565313233";

constexpr std::string_view INIT_EPHEMERAL_SECRET_HEX = // 32 bytes
    "893e28b9dc6ca8d611ab664754b8ceb7bac5117349a4439a6b0569da977c464a";

constexpr std::string_view RESP_STATIC_SECRET_HEX = // 32 bytes
    "4a3acbfdb163dec651dfa3194dece676d437029c62a408b4c5ea9114246e4893";

constexpr std::string_view EXPECTED_RESP_STATIC_PUBLIC_HEX = // 32 bytes
    "31e0303fd6418d2f8c0e78b91f22e8caed0fbe48656dcf4767e4834f701b8f62";

constexpr std::string_view RESP_EPHEMERAL_SECRET_HEX = // 32 bytes
    "bbdb4cdbd309f1a1f2e1456967fe288cadd6f712d65dc7b7793d5e63da6b375b";

constexpr std::string_view EXPECTED_HANDSHAKE_HASH_HEX = // 32 bytes
    "d5c4ce9ffe8bcd940aa50f842a5d4d90d3f7163f4b3916deb87a5d747712d718";

struct wire_message {
    std::string_view payload_hex;
    std::string_view ciphertext_hex;
};

// messages[0] and [1] are the two handshake messages (e, es then e, ee).
// messages[2..5] are transport messages, alternating I -> R -> I -> R.
constexpr std::array<wire_message, 6> MESSAGES{{
    wire_message{
        "4c756477696720766f6e204d69736573",
        "ca35def5ae56cec33dc2036731ab14896bc4c75dbb07a61f879f8e3afa4c79448134d0"
        "0711fdb390a0d178fa008f6d47f49a76e297aa164052a3d842aa8ff7d8",
    },
    wire_message{
        "4d757272617920526f746862617264",
        "95ebc60d2b1fa672c1f46a8aa265ef51bfe38e7ccb39ec5be34069f1448088438ea16e"
        "3701bc0d77744f117bee22451628b075da65d4114bd343e2d93006c4",
    },
    wire_message{
        "462e20412e20486179656b",
        "a62de29ce27cb80245d440d986ed816c156e9d757d7008df2198b0",
    },
    wire_message{
        "4361726c204d656e676572",
        "174a35f11c689f4530d7208618e0564ae12f2f50ba8eb4df5382ff",
    },
    wire_message{
        "4a65616e2d426170746973746520536179",
        "337e475ebb8eae60f91974c4e455a5af38d1d8628d1803b160d60442874b0a1777",
    },
    wire_message{
        "457567656e2042f6686d20766f6e2042617765726b",
        "047e80e060b7bb08b53c5a23dfe9920cae135b9d1dc6302fc475003062723700366346"
        "ac9d",
    },
}};

} // namespace hypercom::tests::noise_vector
