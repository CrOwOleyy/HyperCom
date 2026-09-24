#pragma once

#include <cstddef>
#include <cstdint>

namespace hypercom::proto {

// All protocol bounds, in one place. They're constexpr: no mutable
// global (rule G4), so no limit is tunable at runtime. Whatever varies
// per deployment lives in hypercom.conf and gets passed in as a
// parameter.

constexpr std::uint16_t PROTOCOL_VERSION = 1;

// Framing: [u32 body_size][u8 type][payload]
// body_size counts the type byte AND the payload, but not the field itself.
constexpr std::size_t FRAME_LENGTH_FIELD_SIZE = 4;
constexpr std::size_t FRAME_TYPE_FIELD_SIZE = 1;
constexpr std::size_t FRAME_HEADER_SIZE =
    FRAME_LENGTH_FIELD_SIZE + FRAME_TYPE_FIELD_SIZE;

// Hard cap, checked BEFORE any allocation.
constexpr std::size_t MAX_FRAME_SIZE = 1024 * 1024;
constexpr std::size_t MAX_BODY_SIZE = MAX_FRAME_SIZE - FRAME_LENGTH_FIELD_SIZE;
constexpr std::size_t MAX_PAYLOAD_SIZE = MAX_FRAME_SIZE - FRAME_HEADER_SIZE;

// Cryptographic sizes, duplicated here so the parser stays independent
// of libsodium and therefore fuzzable in isolation.
constexpr std::size_t PUBLIC_KEY_SIZE = 32;
constexpr std::size_t SIGNATURE_SIZE = 64;
constexpr std::size_t AUTH_NONCE_SIZE = 32;

// Application-level bounds. 16 KiB for a post: plenty to write a long
// entry, and low enough that a whole thread still fits in a frame.
constexpr std::size_t MAX_HANDLE_LENGTH = 32;
constexpr std::size_t MAX_FORUM_NAME_LENGTH = 48;
constexpr std::size_t MAX_FORUM_DESCRIPTION_LENGTH = 512;
constexpr std::size_t MAX_POST_TITLE_LENGTH = 200;
constexpr std::size_t MAX_POST_BODY_LENGTH = 16 * 1024;
// Preview served in a post list. Without truncation, a page of 200 posts
// would pull 3 MiB and blow past the frame on its own.
constexpr std::size_t MAX_POST_PREVIEW_LENGTH = 280;
constexpr std::size_t MAX_COMMENT_BODY_LENGTH = 8 * 1024;
constexpr std::size_t MAX_DISPLAY_NAME_LENGTH = 48;
constexpr std::size_t MAX_BIO_LENGTH = 2048;
constexpr std::size_t MAX_THEME_JSON_LENGTH = 1024;
constexpr std::size_t MAX_BLOB_REFERENCE_LENGTH = 96;
constexpr std::size_t MAX_ERROR_MESSAGE_LENGTH = 256;
constexpr std::size_t MAX_MOTD_LENGTH = 4096;
// Report reason. Short by design: it's a signal for the admin, not an
// incident report.
constexpr std::size_t MAX_REPORT_REASON_LENGTH = 500;

// DM envelope: opaque to the server, but bounded like everything else.
constexpr std::size_t MAX_DM_CIPHERTEXT_SIZE = 64 * 1024;

// Pagination: a list response never exceeds this number of elements,
// no matter what value the client requests.
constexpr std::uint16_t MAX_LIST_ITEMS = 200;
constexpr std::uint16_t DEFAULT_LIST_ITEMS = 50;
constexpr std::uint16_t MAX_THREAD_COMMENTS = 500;
constexpr std::uint16_t MAX_DM_BATCH_ITEMS = 100;
constexpr std::uint16_t MAX_FRIEND_ITEMS = 500;

constexpr std::uint8_t TOP8_SLOT_COUNT = 8;

constexpr std::uint16_t DEFAULT_CLEARNET_PORT = 7717;
constexpr std::uint16_t DEFAULT_ONION_PORT = 7718;

} // namespace hypercom::proto
