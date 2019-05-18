#include <shift/rc/image_util/image.hpp>
#include <shift/rc/image_util/convert.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <array>

using namespace shift;
using namespace shift::rc::image_util;
using namespace shift::resource_db;

template <typename... Ts>
constexpr auto make_byte_array(Ts... values) noexcept
  -> std::array<std::byte, sizeof...(Ts)>
{
  return {static_cast<std::byte>(std::forward<Ts>(values))...};
}

template <typename SourceReference, typename DestinationReference>
void test_convert(const SourceReference& source_reference,
                  image_format source_format,
                  const DestinationReference& destination_reference,
                  image_format destination_format)
{
  std::vector<std::byte> destination_buffer;
  destination_buffer.resize(destination_reference.size(), std::byte{0x77});

  destination_image_descriptor destination_image;
  destination_image.width = 1;
  destination_image.height = 1;
  destination_image.row_stride = 0;
  destination_image.format = destination_format;
  destination_image.buffer_size = destination_buffer.size();
  destination_image.buffer = destination_buffer.data();
  source_image_descriptor source_image;
  source_image.width = 1;
  source_image.height = 1;
  source_image.row_stride = 0;
  source_image.format = source_format;
  source_image.buffer_size = source_reference.size();
  source_image.buffer = source_reference.data();
  convert_region region;
  region.destination_x = 0;
  region.destination_y = 0;
  region.source_x = 0;
  region.source_y = 0;
  region.width = 1;
  region.height = 1;
  convert_image(destination_image, source_image, region);

  BOOST_CHECK_EQUAL(
    std::memcmp(destination_buffer.data(), destination_reference.data(),
                destination_reference.size()),
    0);
}

template <typename SourceReference, typename DestinationReference>
void test_convert_bidirectional(
  const SourceReference& source_reference, image_format source_format,
  const DestinationReference& destination_reference,
  image_format destination_format)
{
  test_convert(source_reference, source_format, destination_reference,
               destination_format);
  test_convert(destination_reference, destination_format, source_reference,
               source_format);
}

BOOST_AUTO_TEST_CASE(rc_image_util_copy)
{
  test_convert(
    make_byte_array(0x11, 0x22, 0x33, 0x44), image_format::r8g8b8a8_unorm,
    make_byte_array(0x11, 0x22, 0x33, 0x44), image_format::r8g8b8a8_unorm);
  test_convert(make_byte_array(0x01, 0x10, 0x02, 0x20, 0x03, 0x30, 0x04, 0x40),
               image_format::r16g16b16a16_unorm,
               make_byte_array(0x01, 0x10, 0x02, 0x20, 0x03, 0x30, 0x04, 0x40),
               image_format::r16g16b16a16_unorm);
}

BOOST_AUTO_TEST_CASE(rc_image_util_swizzle)
{
  test_convert(
    make_byte_array(0x11, 0x22, 0x33, 0x44), image_format::r8g8b8a8_unorm,
    make_byte_array(0x33, 0x22, 0x11, 0x44), image_format::b8g8r8a8_unorm);
  /// ToDo: image_format::b16g16r16a16_unorm does not exist.
  // test_convert(
  //   make_byte_array(0x01, 0x10, 0x02, 0x20, 0x03, 0x30, 0x04, 0x40),
  //   image_format::r16g16b16a16_unorm,
  //   make_byte_array(0x03, 0x30, 0x02, 0x20, 0x01, 0x10, 0x04, 0x40),
  //   image_format::b16g16r16a16_unorm);
}

BOOST_AUTO_TEST_CASE(rc_image_util_skip)
{
  test_convert(make_byte_array(0x11, 0x22, 0x33, 0x44),
               image_format::r8g8b8a8_unorm, make_byte_array(0x11, 0x22, 0x33),
               image_format::r8g8b8_unorm);
  test_convert(make_byte_array(0x01, 0x10, 0x02, 0x20, 0x03, 0x30, 0x04, 0x40),
               image_format::r16g16b16a16_unorm,
               make_byte_array(0x01, 0x10, 0x02, 0x20, 0x03, 0x30),
               image_format::r16g16b16_unorm);
}

BOOST_AUTO_TEST_CASE(rc_image_util_fill)
{
  test_convert(make_byte_array(0x11, 0x22, 0x33), image_format::r8g8b8_unorm,
               make_byte_array(0x11, 0x22, 0x33, 0x00),
               image_format::r8g8b8a8_unorm);
  test_convert(make_byte_array(0x01, 0x10, 0x02, 0x20, 0x03, 0x30),
               image_format::r16g16b16_unorm,
               make_byte_array(0x01, 0x10, 0x02, 0x20, 0x03, 0x30, 0x00, 0x00),
               image_format::r16g16b16a16_unorm);
}

BOOST_AUTO_TEST_CASE(rc_image_util_convert_sign)
{
  test_convert_bidirectional(make_byte_array(0x00), image_format::r8_unorm,
                             make_byte_array(0x80), image_format::r8_snorm);
  test_convert_bidirectional(make_byte_array(0xFF), image_format::r8_unorm,
                             make_byte_array(0x7F), image_format::r8_snorm);
  test_convert_bidirectional(make_byte_array(0x11), image_format::r8_unorm,
                             make_byte_array(0x91), image_format::r8_snorm);

  test_convert_bidirectional(
    make_byte_array(0x00, 0x00), image_format::r16_unorm,
    make_byte_array(0x00, 0x80), image_format::r16_snorm);
  test_convert_bidirectional(
    make_byte_array(0xFF, 0xFF), image_format::r16_unorm,
    make_byte_array(0xFF, 0x7F), image_format::r16_snorm);
  test_convert_bidirectional(
    make_byte_array(0x11, 0x22), image_format::r16_unorm,
    make_byte_array(0x11, 0xA2), image_format::r16_snorm);
}

BOOST_AUTO_TEST_CASE(rc_image_util_convert_bits)
{
  test_convert_bidirectional(make_byte_array(0x00, 0x00),
                             image_format::r16_unorm, make_byte_array(0x00),
                             image_format::r8_unorm);
  test_convert_bidirectional(make_byte_array(0x33, 0x33),
                             image_format::r16_unorm, make_byte_array(0x33),
                             image_format::r8_unorm);
  test_convert_bidirectional(make_byte_array(0x99, 0x99),
                             image_format::r16_unorm, make_byte_array(0x99),
                             image_format::r8_unorm);
  test_convert_bidirectional(make_byte_array(0xFF, 0xFF),
                             image_format::r16_unorm, make_byte_array(0xFF),
                             image_format::r8_unorm);
}
