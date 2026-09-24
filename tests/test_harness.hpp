#pragma once

#include <cstdint>

namespace hypercom::tests {

// Minimal, hand-written test harness.
//
// The project rules out any dependency beyond libsodium, SQLite, and
// ImGui. A test framework would add a fourth one, for a need that fits in
// three functions: count, report, summarize.
class test_report {
public:
    test_report();

    void record_check(bool passed, char const *expression, char const *file,
                      int line);

    // Returns the process exit code: 0 if everything passes.
    [[nodiscard]] int summarize(char const *suite_name) const;

private:
    std::uint32_t checks_run_;
    std::uint32_t checks_failed_;
};

} // namespace hypercom::tests

#define HYPERCOM_CHECK(report, expression)                                     \
    (report).record_check((expression), #expression, __FILE__, __LINE__)
