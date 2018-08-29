#include <shift/crypto/salsa20.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <array>

BOOST_AUTO_TEST_CASE(crypto_salsa20)
{
  using namespace shift::crypto;

  salsa20 salsa20;
  salsa20.key() = salsa20::key_t{
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
     0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
     0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f}};

  salsa20::nonce_t nonce{{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                          0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f}};

  std::vector<char> input;
  input.resize(200);
  for (std::size_t i = 0; i < input.size(); ++i)
    input[i] = static_cast<char>(i);
  std::vector<char> output = input;
  salsa20.transform(nonce, output);
}
