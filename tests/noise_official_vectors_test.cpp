#include "common/crypto/sodium_runtime.hpp"
#include "tests/noise_vector_replay.hpp"
#include "tests/test_harness.hpp"

// Validation contre un vecteur de test officiel du projet Noise. Source et
// details de la sequence rejouee : tests/noise_vector_replay.hpp.
//
// crypto_round_trip_test verifie que notre initiateur et notre repondeur sont
// d'accord entre eux, ce qui n'attrape pas un bug present a l'identique des
// deux cotes -- ils se comprendraient parfaitement en se trompant pareil.
// Comparer nos octets a une reference externe est le seul moyen de couvrir ce
// cas.

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
