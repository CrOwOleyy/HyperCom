#include "common/crypto/sodium_runtime.hpp"
#include "tests/noise_vector_replay.hpp"
#include "tests/test_harness.hpp"

// Validation against an official test vector from the Noise project. Source
// and details of the replayed sequence: tests/noise_vector_replay.hpp.
//
// crypto_round_trip_test checks that our initiator and our responder agree
// with each other, which doesn't catch a bug present identically on both
// sides -- they'd understand each other perfectly while being wrong the
// same way. Comparing our bytes against an external reference is the only
// way to cover that case.

int main()
{
    if (!hypercom::crypto::initialize_sodium()) {
        return 1;
    }
    hypercom::tests::test_report report;
    hypercom::tests::loaded_vector_keys const keys =
        hypercom::tests::load_vector_keys(report);
    hypercom::tests::handshake_result const handshake =
        hypercom::tests::run_handshake(report, keys);
    hypercom::tests::check_transport(report, handshake);
    return report.summarize("vecteurs officiels Noise");
}
