#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/frame_codec.hpp"
#include "common/protocol/register_message.hpp"
#include "common/protocol/text_field_codec.hpp"
#include "tests/test_harness.hpp"

#include <cstdint>
#include <vector>

namespace {

using namespace hypercom;

// The reader must never run past its buffer, no matter what the input is.
void check_reader_bounds(tests::test_report &report)
{
    std::vector<std::uint8_t> const empty;
    proto::byte_reader reader{empty};
    std::uint32_t value = 0;
    HYPERCOM_CHECK(report, !reader.read_integer(value));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    std::vector<std::uint8_t> const partial{0x01, 0x02, 0x03};
    proto::byte_reader short_reader{partial};
    HYPERCOM_CHECK(report, !short_reader.read_integer(value));
    // A failure must consume nothing: the cursor stays where it was.
    HYPERCOM_CHECK(report, short_reader.count_remaining_bytes() == 3);
}

// A huge announced length must trigger no allocation at all, only a
// rejection. This is the parser's single most important property.
void check_length_prefix_cap(tests::test_report &report)
{
    std::vector<std::uint8_t> hostile{0xFF, 0xFF, 0xFF, 0xFF};
    proto::byte_reader reader{hostile};
    std::vector<std::uint8_t> destination;
    HYPERCOM_CHECK(report, !reader.read_length_prefixed(destination, 1024));
    HYPERCOM_CHECK(report, destination.empty());
    std::vector<std::uint8_t> announced_beyond_buffer{0x10, 0x00, 0x00, 0x00,
                                                      0x41};
    proto::byte_reader second{announced_beyond_buffer};
    HYPERCOM_CHECK(report, !second.read_length_prefixed(destination, 1024));
}

void check_round_trip(tests::test_report &report)
{
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    writer.write_integer(static_cast<std::uint64_t>(0x0123456789ABCDEFULL));
    proto::write_text_field(writer, "bonjour");
    proto::byte_reader reader{buffer};
    std::uint64_t recovered = 0;
    std::string text;
    HYPERCOM_CHECK(report, reader.read_integer(recovered));
    HYPERCOM_CHECK(report, recovered == 0x0123456789ABCDEFULL);
    HYPERCOM_CHECK(report, proto::read_text_field(reader, text, 32) &&
                               text == "bonjour");
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
}

void check_utf8_validation(tests::test_report &report)
{
    HYPERCOM_CHECK(report, proto::validate_text_field("texte normal"));
    HYPERCOM_CHECK(report, proto::validate_text_field("accentue : eee"));
    HYPERCOM_CHECK(report, proto::validate_text_field("\xC3\xA9"));
    // Overlong: 0xC0 0x80 encodes U+0000 over two bytes.
    HYPERCOM_CHECK(report, !proto::validate_text_field("\xC0\x80"));
    // Surrogate half U+D800.
    HYPERCOM_CHECK(report, !proto::validate_text_field("\xED\xA0\x80"));
    // Isolated continuation byte.
    HYPERCOM_CHECK(report, !proto::validate_text_field("\x80"));
    // C0 control characters rejected, except tab and newline.
    HYPERCOM_CHECK(report, !proto::validate_text_field(std::string(1, '\0')));
    HYPERCOM_CHECK(report, !proto::validate_text_field("\r"));
    HYPERCOM_CHECK(report, proto::validate_text_field("a\tb\nc"));
}

void check_frame_codec(tests::test_report &report)
{
    std::vector<std::uint8_t> const payload{0xAA, 0xBB};
    std::vector<std::uint8_t> frame;
    HYPERCOM_CHECK(
        report,
        proto::encode_frame(proto::message_type::ping_request, payload, frame));
    proto::frame_header header{};
    HYPERCOM_CHECK(report, proto::decode_frame_header(frame, header));
    HYPERCOM_CHECK(report, header.type == proto::message_type::ping_request);
    HYPERCOM_CHECK(report, header.body_size == payload.size() + 1);
    // Unknown type: rejected at the header, never passed to a handler.
    std::vector<std::uint8_t> forged{0x02, 0x00, 0x00, 0x00, 0xFE, 0x00};
    HYPERCOM_CHECK(report, !proto::decode_frame_header(forged, header));
    // Zero length: a frame always contains at least its type.
    std::vector<std::uint8_t> const zero_length{0x00, 0x00, 0x00, 0x00, 0x06};
    HYPERCOM_CHECK(report, !proto::decode_frame_header(zero_length, header));
}

void check_handle_validation(tests::test_report &report)
{
    HYPERCOM_CHECK(report, proto::validate_handle("alice"));
    HYPERCOM_CHECK(report, proto::validate_handle("a_b-1"));
    HYPERCOM_CHECK(report, !proto::validate_handle("ab"));
    HYPERCOM_CHECK(report, !proto::validate_handle("1abc"));
    // Uppercase allowed (e.g. "Alice")
    HYPERCOM_CHECK(report, proto::validate_handle("Alice"));
    // Cyrillic homoglyph rejected by the ASCII restriction.
    HYPERCOM_CHECK(report, !proto::validate_handle("\xD1\x83ounes"));
}

} // namespace

int main()
{
    hypercom::tests::test_report report;
    check_reader_bounds(report);
    check_length_prefix_cap(report);
    check_round_trip(report);
    check_utf8_validation(report);
    check_frame_codec(report);
    check_handle_validation(report);
    return report.summarize("protocole");
}
