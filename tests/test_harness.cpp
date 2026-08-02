#include "tests/test_harness.hpp"

#include <iostream>

namespace hypercom::tests {

test_report::test_report() : checks_run_{0}, checks_failed_{0} {}

void test_report::record_check(bool passed, char const *expression,
                               char const *file, int line)
{
    ++checks_run_;
    if (passed) {
        return;
    }
    ++checks_failed_;
    std::cerr << "  ECHEC " << file << ':' << line << " : " << expression
              << '\n';
}

int test_report::summarize(char const *suite_name) const
{
    std::cout << suite_name << " : " << (checks_run_ - checks_failed_) << '/'
              << checks_run_ << " verifications passees\n";
    return checks_failed_ == 0 ? 0 : 1;
}

} // namespace hypercom::tests
