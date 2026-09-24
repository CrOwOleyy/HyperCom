#include "tests/test_harness.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

// Replays the fuzzing corpus without libFuzzer.
//
// The tools/fuzz harness requires clang. This test calls the same entry
// point from the regular suite, so already-known edge cases get checked on
// every build -- including under gcc, and under the sanitizers enabled by
// HYPERCOM_SANITIZER.
//
// It doesn't replace an actual fuzzing campaign: it only prevents a
// regression on a case we already paid the price to find.

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

// Success is the absence of a crash: if the parser overruns, the process
// dies here and ctest reports it. So there's nothing to compare.
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
