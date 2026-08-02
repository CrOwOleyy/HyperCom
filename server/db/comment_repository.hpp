#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "common/protocol/content_records.hpp"
#include "server/db/database_handle.hpp"

namespace hypercom::server {

// Profondeur maximale d'imbrication. Sans plafond, une chaine de reponses
// suffisamment longue ferait exploser la requete recursive et le rendu client.
constexpr std::int64_t MAX_COMMENT_DEPTH = 24;

class comment_repository {
public:
    explicit comment_repository(database_handle &database);

    [[nodiscard]] bool create_comment(std::int64_t post_id,
                                      std::int64_t parent_comment_id,
                                      std::int64_t author_id,
                                      std::string_view body,
                                      std::int64_t &out_id);

    [[nodiscard]] bool find_by_id(std::int64_t id,
                                  proto::comment_record &out);

    // Renvoie l'arbre a plat, en parcours prefixe, chaque element portant sa
    // profondeur. Le C++ ne recurse jamais : c'est WITH RECURSIVE qui parcourt,
    // avec une borne de profondeur dans la requete elle-meme.
    [[nodiscard]] bool list_thread(std::int64_t post_id, std::uint16_t limit,
                                   std::vector<proto::comment_record> &out,
                                   bool &truncated);

    // Verifie qu'un parent existe ET appartient bien au meme post, avant
    // d'accepter une reponse. Sans ce controle, un client pourrait greffer un
    // commentaire sous le fil de quelqu'un d'autre.
    [[nodiscard]] bool check_parent_belongs_to_post(
        std::int64_t parent_comment_id, std::int64_t post_id);

private:
    database_handle &database_;
};

} // namespace hypercom::server
