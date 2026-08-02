#pragma once

#include <cstdint>

namespace hypercom::tests {

// Harnais de test minimal, ecrit a la main.
//
// Le projet s'interdit d'autres dependances que libsodium, SQLite et ImGui.
// Un framework de test en ferait une quatrieme, pour un besoin qui tient en
// trois fonctions : compter, signaler, resumer.
class test_report {
public:
    test_report();

    void record_check(bool passed, char const *expression, char const *file,
                      int line);

    // Renvoie le code de sortie du processus : 0 si tout passe.
    [[nodiscard]] int summarize(char const *suite_name) const;

private:
    std::uint32_t checks_run_;
    std::uint32_t checks_failed_;
};

} // namespace hypercom::tests

#define HYPERCOM_CHECK(report, expression)                                    \
    (report).record_check((expression), #expression, __FILE__, __LINE__)
