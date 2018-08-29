#include <shift/math/utility.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>

using namespace shift::math;

BOOST_AUTO_TEST_CASE(utility_is_power_of_two)
{
  static_assert(!is_power_of_two(0x00000000u), "Error in is_power_of_two");
  static_assert(is_power_of_two(0x00000001u), "Error in is_power_of_two");
  static_assert(is_power_of_two(0x00000002u), "Error in is_power_of_two");
  static_assert(!is_power_of_two(0x00000003u), "Error in is_power_of_two");
  static_assert(is_power_of_two(0x80000000u), "Error in is_power_of_two");
}

BOOST_AUTO_TEST_CASE(utility_next_power_of_two)
{
  static_assert(next_power_of_two(0x00000100u) == 0x00000200u,
                "Error in next_power_of_two.");
  static_assert(next_power_of_two(0x00001001u) == 0x00002000u,
                "Error in next_power_of_two.");
  static_assert(next_power_of_two(0x00110111u) == 0x00200000u,
                "Error in next_power_of_two.");
  static_assert(next_power_of_two(0x01ffffffu) == 0x02000000u,
                "Error in next_power_of_two.");
  static_assert(next_power_of_two(0x40000000u) == 0x80000000u,
                "Error in next_power_of_two.");
  static_assert(next_power_of_two(0x00000000u) == 0x00000001u,
                "Error in next_power_of_two.");
  static_assert(next_power_of_two(0x80000000u) == 0x00000000u,
                "Error in next_power_of_two.");
}

template <typename T>
void test_NaNs()
{
  if constexpr (std::numeric_limits<T>::has_quiet_NaN)
    BOOST_CHECK(is_nan(quiet_nan<T>()));
  else
    BOOST_CHECK(!is_nan(quiet_nan<T>()));

  if constexpr (std::numeric_limits<T>::has_signaling_NaN)
    BOOST_CHECK(is_nan(signaling_nan<T>()));
  else
    BOOST_CHECK(!is_nan(signaling_nan<T>()));

  if constexpr (std::numeric_limits<T>::has_quiet_NaN &&
                std::numeric_limits<T>::has_signaling_NaN)
  {
    // Any two NaN values are considered equal, regardless of whether they're
    // signalling or quiet.
    BOOST_CHECK_EQUAL(quiet_nan<T>(), signaling_nan<T>());

    // However, they should be different in binary form.
    auto qnan = quiet_nan<T>();
    auto snan = signaling_nan<T>();
    BOOST_CHECK(std::memcmp(&qnan, &snan, sizeof(T)) != 0);
  }
}

BOOST_AUTO_TEST_CASE(utility_float_NaNs)
{
  test_NaNs<float>();
  test_NaNs<double>();
  test_NaNs<long double>();
}

BOOST_AUTO_TEST_CASE(utility_integer_NaNs)
{
  test_NaNs<char>();
  test_NaNs<short>();
  test_NaNs<int>();
}

BOOST_AUTO_TEST_CASE(utility_literals)
{
  using namespace shift::math::literals;

  BOOST_CHECK_EQUAL(static_cast<float>(180_deg), pi<float>);
  BOOST_CHECK_EQUAL(180_deg, pi<double>);
  BOOST_CHECK_EQUAL(180_fdeg, pi<float>);

  BOOST_CHECK_EQUAL(static_cast<float>(180.0_deg), pi<float>);
  BOOST_CHECK_EQUAL(180.0_deg, pi<double>);
  BOOST_CHECK_EQUAL(180.0_fdeg, pi<float>);
}
