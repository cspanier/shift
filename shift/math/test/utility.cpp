#include <shift/math/utility.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>

using namespace shift::math;

BOOST_AUTO_TEST_CASE(utility_almost_equal)
{
  const auto value1 = 0.2;
  const auto value2 = 1.0 / std::sqrt(5.0) / std::sqrt(5.0);

  BOOST_CHECK_NE(value1, value2);
  BOOST_CHECK(almost_equal(value1, value2));
}

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
    // Comparison of any value with a NaN value returns false.
    BOOST_CHECK_NE(quiet_nan<T>(), T{0});
    BOOST_CHECK_NE(signaling_nan<T>(), T{0});

    // Comparison of two identical NaN values should also return false.
    /// ToDo: Visual C++ fails here. Find the reason why.
    BOOST_CHECK_NE(quiet_nan<T>(), quiet_nan<T>());
    BOOST_CHECK_NE(signaling_nan<T>(), signaling_nan<T>());

    // Comparison of different NaN values should return false as well.
    BOOST_CHECK_NE(quiet_nan<T>(), signaling_nan<T>());

    // Signalling and quiet NaN values should be different in binary
    // representation.
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
