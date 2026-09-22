#include "client/keystore/server_registry.hpp"

#include <algorithm>
#include <filesystem>

#include "client/keystore/sealed_file.hpp"
#include "common/crypto/keystore_envelope.hpp"
#include "common/crypto/secure_memory.hpp"
#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/text_field_codec.hpp"

namespace hypercom::client {
namespace {

constexpr std::uint8_t REGISTRY_VERSION = 1;
constexpr std::size_t MAX_LABEL_LENGTH = 48;
constexpr std::size_t MAX_HOST_LENGTH = 256;
constexpr std::size_t MAX_PATH_LENGTH = 512;
constexpr std::uint16_t MAX_SERVERS = 64;

void write_entry(proto::byte_writer &writer, server_entry const &entry)
{
    proto::write_text_field(writer, entry.label);
    proto::write_text_field(writer, entry.endpoint.host);
    writer.write_integer(entry.endpoint.port);
    proto::write_text_field(writer, entry.endpoint.socks5_host);
    writer.write_integer(entry.endpoint.socks5_port);
    writer.write_fixed_bytes(entry.server_key);
    writer.write_integer(static_cast<std::uint8_t>(entry.source));
    proto::write_text_field(writer, entry.imported_identity_path);
    writer.write_integer(static_cast<std::uint8_t>(entry.trust_acknowledged));
}

// Le fichier vient du disque de l'utilisateur, mais il est lu avec la meme
// rigueur qu'une trame reseau : un disque corrompu ne doit pas mieux s'en tirer
// qu'un pair hostile.
[[nodiscard]] bool read_entry(proto::byte_reader &reader, server_entry &out)
{
    std::uint8_t raw_source = 0;
    std::uint8_t raw_acknowledged = 0;
    if (!proto::read_text_field(reader, out.label, MAX_LABEL_LENGTH)
        || !proto::read_text_field(reader, out.endpoint.host, MAX_HOST_LENGTH)
        || !reader.read_integer(out.endpoint.port)
        || !proto::read_text_field(reader, out.endpoint.socks5_host,
                                   MAX_HOST_LENGTH)
        || !reader.read_integer(out.endpoint.socks5_port)
        || !reader.read_fixed_bytes(out.server_key)
        || !reader.read_integer(raw_source)
        || !proto::read_text_field(reader, out.imported_identity_path,
                                   MAX_PATH_LENGTH)
        || !reader.read_integer(raw_acknowledged)) {
        return false;
    }
    if (raw_source > static_cast<std::uint8_t>(identity_source::imported)) {
        return false;
    }
    out.source = static_cast<identity_source>(raw_source);
    out.trust_acknowledged = raw_acknowledged != 0;
    return true;
}

[[nodiscard]] bool parse_entries(std::span<std::uint8_t const> plaintext,
                                 std::vector<server_entry> &out)
{
    proto::byte_reader reader{plaintext};
    std::uint8_t version = 0;
    std::uint16_t count = 0;
    if (!reader.read_integer(version) || version != REGISTRY_VERSION
        || !reader.read_integer(count) || count > MAX_SERVERS) {
        return false;
    }
    out.clear();
    out.reserve(count);
    for (std::uint16_t index = 0; index < count; ++index) {
        server_entry entry;
        if (!read_entry(reader, entry)) {
            out.clear();
            return false;
        }
        out.push_back(std::move(entry));
    }
    return true;
}

} // namespace

server_registry::server_registry(std::string path) : path_{std::move(path)} {}

bool server_registry::has_stored_registry() const
{
    std::error_code failure;
    return std::filesystem::is_regular_file(path_, failure);
}

bool server_registry::load(std::string_view passphrase,
                           std::vector<server_entry> &out,
                           std::string &error_out)
{
    std::vector<std::uint8_t> sealed;
    if (!read_sealed_file(path_, sealed)) {
        error_out = "registre des serveurs illisible : " + path_;
        return false;
    }
    std::vector<std::uint8_t> plaintext;
    if (!crypto::open_blob(passphrase, sealed, plaintext)) {
        error_out = "passphrase incorrecte ou registre altere";
        return false;
    }
    bool const parsed = parse_entries(plaintext, out);
    crypto::wipe_bytes(plaintext);
    if (!parsed) {
        error_out = "registre des serveurs mal forme : " + path_;
    }
    return parsed;
}

bool server_registry::save(std::string_view passphrase,
                           std::vector<server_entry> const &entries,
                           std::string &error_out)
{
    if (entries.size() > MAX_SERVERS) {
        error_out = "trop de serveurs enregistres";
        return false;
    }
    std::vector<std::uint8_t> plaintext;
    proto::byte_writer writer{plaintext};
    writer.write_integer(REGISTRY_VERSION);
    writer.write_integer(static_cast<std::uint16_t>(entries.size()));
    for (server_entry const &entry : entries) {
        write_entry(writer, entry);
    }
    std::vector<std::uint8_t> sealed;
    bool const succeeded = crypto::seal_blob(passphrase, plaintext, sealed)
                           && write_sealed_file(path_, sealed, error_out);
    crypto::wipe_bytes(plaintext);
    if (!succeeded && error_out.empty()) {
        error_out = "scellement du registre impossible";
    }
    return succeeded;
}

} // namespace hypercom::client
