#include <shift/core/hex_dump.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <sstream>
#include <cstdint>

BOOST_AUTO_TEST_CASE(hexdump_3of4)
{
  using namespace shift::core;

  // Number of elements in container less than the number of bytes per line.
  std::stringstream s;
  std::vector<std::uint8_t> v{100, 200, 44};
  s << hex_dump<4>(v);
  BOOST_CHECK_EQUAL(s.str(), "64 c8 2c    d.,\n");
}

BOOST_AUTO_TEST_CASE(hexdump_4of4)
{
  using namespace shift::core;

  // Number of elements in container equal to the number of bytes per line.
  std::stringstream s;
  std::vector<std::uint8_t> v{100, 200, 44, 113};
  s << hex_dump<4>(v);
  BOOST_CHECK_EQUAL(s.str(), "64 c8 2c 71 d.,q\n");
}

BOOST_AUTO_TEST_CASE(hexdump_5of4)
{
  using namespace shift::core;

  // Number of elements in container larger than the number of bytes per line.
  std::stringstream s;
  std::vector<std::uint8_t> v{100, 200, 44, 113, 16};
  s << hex_dump<4>(v);
  BOOST_CHECK_EQUAL(s.str(), "64 c8 2c 71 d.,q\n10          .\n");
}
