#include "client/net/server_connection.hpp"

#include "common/crypto/key_types.hpp"
#include "common/crypto/secure_memory.hpp"
#include "common/protocol/length_prefixed_stream.hpp"
#include "common/protocol/protocol_limits.hpp"

namespace hypercom::client {
namespace {

constexpr std::size_t MAX_NOISE_MESSAGE_SIZE =
    proto::MAX_FRAME_SIZE + crypto::AEAD_TAG_SIZE;
// Le delai de lecture de la socket est de 200 ms : cinquante tours donnent une
// dizaine de secondes avant d'abandonner une reponse.
constexpr int MAX_RECEIVE_ATTEMPTS = 50;

} // namespace

server_connection::server_connection(
    crypto::x25519_public_key const &server_static_public)
    : socket_{}, server_static_public_{server_static_public},
      handshake_{server_static_public}, transport_{}, input_buffer_{},
      open_{false}
{
}

bool server_connection::is_open() const
{
    return open_;
}

bool server_connection::pull_next_message(std::vector<std::uint8_t> &out,
                                          std::string &error_out)
{
    for (int attempt = 0; attempt < MAX_RECEIVE_ATTEMPTS; ++attempt) {
        bool malformed = false;
        if (proto::extract_length_prefixed_message(
                input_buffer_, MAX_NOISE_MESSAGE_SIZE, out, malformed)) {
            return true;
        }
        if (malformed) {
            error_out = "trame invalide recue du serveur";
            open_ = false;
            return false;
        }
        bool received_any = false;
        if (!socket_.receive_available(input_buffer_, received_any)) {
            error_out = "connexion fermee par le serveur";
            open_ = false;
            return false;
        }
    }
    error_out = "aucune reponse du serveur";
    return false;
}

bool server_connection::perform_handshake(std::string &error_out)
{
    std::vector<std::uint8_t> first_message;
    if (!handshake_.write_first_message({}, first_message)) {
        error_out = "handshake : premier message impossible a construire";
        return false;
    }
    std::vector<std::uint8_t> wire;
    proto::append_length_prefixed_message(first_message, wire);
    if (!socket_.send_all(wire)) {
        error_out = "handshake : envoi impossible";
        return false;
    }
    std::vector<std::uint8_t> response;
    if (!pull_next_message(response, error_out)) {
        return false;
    }
    std::vector<std::uint8_t> payload;
    if (!handshake_.read_second_message(response, payload)) {
        // Cause la plus probable : la cle epinglee ne correspond pas a celle
        // du serveur en face. C'est exactement ce que l'epinglage doit
        // detecter, et il n'y a rien a rattraper.
        error_out = "handshake refuse : la cle du serveur ne correspond pas "
                    "a celle epinglee";
        return false;
    }
    crypto::symmetric_key send_key{};
    crypto::symmetric_key receive_key{};
    if (!handshake_.export_transport_keys(send_key, receive_key)) {
        error_out = "handshake : derivation des cles de transport impossible";
        return false;
    }
    transport_.emplace(send_key, receive_key);
    crypto::wipe_bytes(send_key);
    crypto::wipe_bytes(receive_key);
    return true;
}

bool server_connection::open_session(std::string const &host,
                                     std::uint16_t port,
                                     std::string &error_out)
{
    // Rien de la session precedente ne survit : handshake neuf, canal neuf,
    // tampon vide. Reutiliser le moindre etat rendrait la reconnexion
    // correlable a la connexion d'avant.
    open_ = false;
    transport_.reset();
    input_buffer_.clear();
    handshake_ = crypto::noise_handshake_initiator{server_static_public_};
    if (!socket_.connect_to_host(host, port, error_out)) {
        return false;
    }
    open_ = true;
    if (!perform_handshake(error_out)) {
        open_ = false;
        return false;
    }
    return true;
}

bool server_connection::send_frame(proto::message_type type,
                                   std::span<std::uint8_t const> payload)
{
    if (!open_ || !transport_.has_value()) {
        return false;
    }
    std::vector<std::uint8_t> frame;
    if (!proto::encode_frame(type, payload, frame)) {
        return false;
    }
    std::vector<std::uint8_t> sealed;
    if (!transport_->encrypt_message(frame, sealed)) {
        return false;
    }
    std::vector<std::uint8_t> wire;
    proto::append_length_prefixed_message(sealed, wire);
    return socket_.send_all(wire);
}

bool server_connection::receive_frame(proto::frame_header &header,
                                      std::vector<std::uint8_t> &payload,
                                      std::string &error_out)
{
    if (!open_ || !transport_.has_value()) {
        error_out = "session fermee";
        return false;
    }
    std::vector<std::uint8_t> sealed;
    if (!pull_next_message(sealed, error_out)) {
        return false;
    }
    std::vector<std::uint8_t> frame;
    if (!transport_->decrypt_message(sealed, frame)) {
        error_out = "dechiffrement impossible : session compromise";
        open_ = false;
        return false;
    }
    if (!proto::decode_frame_header(frame, header)
        || frame.size() != proto::FRAME_LENGTH_FIELD_SIZE + header.body_size) {
        error_out = "trame mal formee";
        open_ = false;
        return false;
    }
    payload.assign(frame.begin() + proto::FRAME_HEADER_SIZE, frame.end());
    return true;
}

} // namespace hypercom::client
