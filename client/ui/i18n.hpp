#pragma once

namespace hypercom::client {

enum class language { french, english };

// Returns the translation matching the key in the given language.
// If the key doesn't exist, returns the key itself to help with
// debugging.
const char *tr(const char *key, language lang);

} // namespace hypercom::client
