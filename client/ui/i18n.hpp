#pragma once

namespace hypercom::client {

enum class language {
    french,
    english
};

// Renvoie la traduction correspondant a la cle dans la langue donnee.
// Si la cle n'existe pas, renvoie la cle elle-meme pour aider au debug.
const char* tr(const char* key, language lang);

} // namespace hypercom::client
