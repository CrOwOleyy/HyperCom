#include "client/net/socks5_connector.hpp"

#include <vector>

namespace hypercom::client {
namespace {

constexpr std::uint8_t SOCKS_VERSION = 0x05;
constexpr std::uint8_t AUTH_NONE = 0x00;
constexpr std::uint8_t COMMAND_CONNECT = 0x01;
constexpr std::uint8_t ADDRESS_TYPE_DOMAIN = 0x03;
constexpr std::uint8_t ADDRESS_TYPE_IPV4 = 0x01;
constexpr std::uint8_t ADDRESS_TYPE_IPV6 = 0x04;
constexpr std::uint8_t REPLY_SUCCESS = 0x00;
constexpr std::size_t MAX_DOMAIN_LENGTH = 255;

// La socket expire en lecture toutes les 200 ms. Construire un circuit Tor
// prend couramment plusieurs secondes, parfois une trentaine : ce plafond
// laisse le temps au circuit de s'etablir sans bloquer indefiniment.
constexpr int MAX_READ_ATTEMPTS = 150;

[[nodiscard]] std::string describe_reply_code(std::uint8_t code)
{
    switch (code) {
        case 0x01: return "echec general du proxy";
        case 0x02: return "connexion refusee par la regle du proxy";
        case 0x03: return "reseau injoignable";
        case 0x04: return "hote injoignable : l'adresse .onion existe-t-elle ?";
        case 0x05: return "connexion refusee par la cible";
        case 0x06: return "delai expire";
        case 0x07: return "commande non supportee par le proxy";
        case 0x08: return "type d'adresse non supporte par le proxy";
        default: return "code de reponse inconnu";
    }
}

// receive_available n'ajoute que ce qui est disponible : on boucle jusqu'a
// obtenir le compte voulu. Un retour false signale une vraie fermeture, pas
// un simple silence.
[[nodiscard]] bool read_exactly(tcp_client_socket &socket,
                                std::vector<std::uint8_t> &buffer,
                                std::size_t needed, std::string &error_out)
{
    for (int attempt = 0;
         attempt < MAX_READ_ATTEMPTS && buffer.size() < needed; ++attempt) {
        bool received_any = false;
        if (!socket.receive_available(buffer, received_any)) {
            error_out = "proxy SOCKS5 : connexion fermee pendant la negociation";
            return false;
        }
    }
    if (buffer.size() < needed) {
        error_out = "proxy SOCKS5 : aucune reponse. Tor est-il demarre ?";
        return false;
    }
    return true;
}

[[nodiscard]] bool negotiate_no_auth(tcp_client_socket &socket,
                                     std::string &error_out)
{
    std::vector<std::uint8_t> const greeting{SOCKS_VERSION, 0x01, AUTH_NONE};
    if (!socket.send_all(greeting)) {
        error_out = "proxy SOCKS5 : envoi impossible. Tor ecoute-t-il ?";
        return false;
    }
    std::vector<std::uint8_t> reply;
    if (!read_exactly(socket, reply, 2, error_out)) {
        return false;
    }
    if (reply[0] != SOCKS_VERSION || reply[1] != AUTH_NONE) {
        error_out = "proxy SOCKS5 : le proxy exige une authentification";
        return false;
    }
    return true;
}

[[nodiscard]] bool send_connect_request(tcp_client_socket &socket,
                                        std::string const &host,
                                        std::uint16_t port,
                                        std::string &error_out)
{
    if (host.empty() || host.size() > MAX_DOMAIN_LENGTH) {
        error_out = "proxy SOCKS5 : nom d'hote de longueur invalide";
        return false;
    }
    std::vector<std::uint8_t> request{SOCKS_VERSION, COMMAND_CONNECT, 0x00,
                                      ADDRESS_TYPE_DOMAIN,
                                      static_cast<std::uint8_t>(host.size())};
    request.insert(request.end(), host.begin(), host.end());
    // Gros-boutiste, impose par le RFC 1928 -- a l'inverse du reste du
    // protocole HyperCom, qui est petit-boutiste.
    request.push_back(static_cast<std::uint8_t>(port >> 8U));
    request.push_back(static_cast<std::uint8_t>(port & 0xFFU));
    if (!socket.send_all(request)) {
        error_out = "proxy SOCKS5 : envoi de la demande impossible";
        return false;
    }
    return true;
}

// L'adresse liee renvoyee par le proxy est de taille variable selon son type.
// On la lit sans l'utiliser : elle ne sert a rien ici, mais tout octet non
// consomme polluerait le premier message Noise.
[[nodiscard]] std::size_t measure_bound_address(std::uint8_t address_type,
                                                std::uint8_t first_byte)
{
    if (address_type == ADDRESS_TYPE_IPV4) {
        return 4;
    }
    if (address_type == ADDRESS_TYPE_IPV6) {
        return 16;
    }
    if (address_type == ADDRESS_TYPE_DOMAIN) {
        return static_cast<std::size_t>(first_byte) + 1;
    }
    return 0;
}

} // namespace

bool perform_socks5_connect(tcp_client_socket &socket,
                            std::string const &target_host,
                            std::uint16_t target_port, std::string &error_out)
{
    if (!negotiate_no_auth(socket, error_out)
        || !send_connect_request(socket, target_host, target_port,
                                 error_out)) {
        return false;
    }
    std::vector<std::uint8_t> reply;
    if (!read_exactly(socket, reply, 5, error_out)) {
        return false;
    }
    if (reply[0] != SOCKS_VERSION) {
        error_out = "proxy SOCKS5 : reponse non conforme";
        return false;
    }
    if (reply[1] != REPLY_SUCCESS) {
        error_out = "proxy SOCKS5 : " + describe_reply_code(reply[1]);
        return false;
    }
    std::size_t const address_size = measure_bound_address(reply[3], reply[4]);
    if (address_size == 0) {
        error_out = "proxy SOCKS5 : type d'adresse inattendu dans la reponse";
        return false;
    }
    // 4 octets d'entete + l'adresse + 2 octets de port. Le cinquieme octet
    // deja lu fait partie de l'adresse, d'ou le calcul a partir de 4.
    return read_exactly(socket, reply, 4 + address_size + 2, error_out);
}

} // namespace hypercom::client
