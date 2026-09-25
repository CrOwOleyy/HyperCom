# Three libraries rather than one, for a precise reason: hypercom_protocol
# does NOT link libsodium. That's what makes it possible to fuzz the
# parser in complete isolation and to compile it anywhere, including
# where no dependency is installed.

add_library(hypercom_util STATIC
    util/hex_codec.cpp
    util/log_level.cpp
    util/logger.cpp
    util/unix_clock.cpp)
target_include_directories(hypercom_util PUBLIC "${CMAKE_SOURCE_DIR}")
hypercom_apply_strict_options(hypercom_util)
hypercom_apply_sanitizers(hypercom_util)

add_library(hypercom_protocol STATIC
    protocol/auth_message.cpp
    protocol/byte_reader.cpp
    protocol/byte_writer.cpp
    protocol/comment_create_message.cpp
    protocol/content_delete_message.cpp
    protocol/content_records.cpp
    protocol/dm_ack_message.cpp
    protocol/dm_envelope_record.cpp
    protocol/dm_fetch_message.cpp
    protocol/dm_send_message.cpp
    protocol/error_code.cpp
    protocol/forum_create_message.cpp
    protocol/forum_list_message.cpp
    protocol/forum_record.cpp
    protocol/frame_codec.cpp
    protocol/friend_message.cpp
    protocol/hello_message.cpp
    protocol/length_prefixed_stream.cpp
    protocol/message_type.cpp
    protocol/motd_message.cpp
    protocol/ping_message.cpp
    protocol/post_create_message.cpp
    protocol/post_list_message.cpp
    protocol/prekey_fetch_message.cpp
    protocol/prekey_publish_message.cpp
    protocol/profile_get_message.cpp
    protocol/profile_set_message.cpp
    protocol/register_message.cpp
    protocol/report_message.cpp
    protocol/social_records.cpp
    protocol/status_message.cpp
    protocol/text_field_codec.cpp
    protocol/thread_fetch_message.cpp
    protocol/top8_message.cpp)
target_include_directories(hypercom_protocol PUBLIC "${CMAKE_SOURCE_DIR}")
hypercom_apply_strict_options(hypercom_protocol)
hypercom_apply_sanitizers(hypercom_protocol)

add_library(hypercom_crypto STATIC
    crypto/dm_envelope.cpp
    crypto/dm_message_chain.cpp
    crypto/dm_session_keys.cpp
    crypto/hkdf_sha256.cpp
    crypto/identity_keypair.cpp
    crypto/keystore_envelope.cpp
    crypto/noise_cipher_state.cpp
    crypto/noise_handshake_initiator.cpp
    crypto/noise_handshake_responder.cpp
    crypto/noise_payload_codec.cpp
    crypto/noise_symmetric_state.cpp
    crypto/noise_transport.cpp
    crypto/secure_memory.cpp
    crypto/signature_verifier.cpp
    crypto/sodium_runtime.cpp
    crypto/x25519_exchange.cpp)
target_include_directories(hypercom_crypto PUBLIC "${CMAKE_SOURCE_DIR}")
target_link_libraries(hypercom_crypto PUBLIC hypercom::sodium)
hypercom_apply_strict_options(hypercom_crypto)
hypercom_apply_sanitizers(hypercom_crypto)

# Facade for consumers who want everything.
add_library(hypercom_common INTERFACE)
target_link_libraries(hypercom_common INTERFACE
    hypercom_util hypercom_protocol hypercom_crypto)
