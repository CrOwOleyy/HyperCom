#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "tests/test_harness.hpp"

// Rejoue le corpus de fuzzing sans libFuzzer.
//
// Le harnais tools/fuzz exige clang. Ce test appelle le meme point d'entree
// depuis la suite ordinaire, pour que les cas limites deja connus soient
// verifies a chaque build -- y compris sous gcc, et sous les sanitizers
// actives par HYPERCOM_SANITIZER.
//
// Il ne remplace pas une campagne de fuzzing : il empeche seulement une
// regression sur un cas qu'on a deja paye le prix de trouver.

extern "C" int LLVMFuzzerTestOneInput(std::uint8_t const *data,
                                      std::size_t size);

namespace {

using namespace hypercom;

[[nodiscard]] bool read_file(std::filesystem::path const &path,
                             std::vector<std::uint8_t> &out)
{
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        return false;
    }
    out.assign(std::istreambuf_iterator<char>{stream},
               std::istreambuf_iterator<char>{});
    return true;
}

// Le succes est l'absence de plantage : si le parseur deborde, le processus
// meurt ici et ctest le signale. Il n'y a donc rien a comparer.
void replay_corpus(tests::test_report &report)
{
    std::filesystem::path const directory{HYPERCOM_FUZZ_CORPUS_DIR};
    HYPERCOM_CHECK(report, std::filesystem::is_directory(directory));
    std::uint32_t replayed = 0;
    for (auto const &entry : std::filesystem::directory_iterator{directory}) {
        if (!entry.is_regular_file()) {
            continue;
        }
        std::vector<std::uint8_t> content;
        HYPERCOM_CHECK(report, read_file(entry.path(), content));
        static_cast<void>(
            LLVMFuzzerTestOneInput(content.data(), content.size()));
        ++replayed;
    }
    std::cout << "  corpus rejoue : " << replayed << " cas\n";
    HYPERCOM_CHECK(report, replayed > 0);
}

} // namespace

int main()
{
    hypercom::tests::test_report report;
    replay_corpus(report);
    return report.summarize("rejeu du corpus");
}
