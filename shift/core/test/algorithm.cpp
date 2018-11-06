#include <shift/core/algorithm.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <vector>
#include <atomic>
#include <cstdint>

using namespace shift::core;

BOOST_AUTO_TEST_CASE(algorithm_min_max)
{
  BOOST_CHECK_EQUAL(min(1, 2, 3, 4), 1);
  BOOST_CHECK_EQUAL(min(4, 3, 2, 1), 1);
  BOOST_CHECK_EQUAL(min(4, 3, 1, 2), 1);
  BOOST_CHECK_EQUAL(max(1, 2, 3, 4), 4);
  BOOST_CHECK_EQUAL(max(4, 3, 2, 1), 4);
  BOOST_CHECK_EQUAL(max(4, 3, 1, 2), 4);
}

BOOST_AUTO_TEST_CASE(algorithm_clamp)
{
  BOOST_CHECK_EQUAL(clamp(0.5f, 1.0f, 2.0f), 1.0f);
  BOOST_CHECK_EQUAL(clamp(1.5f, 1.0f, 2.0f), 1.5f);
  BOOST_CHECK_EQUAL(clamp(2.5f, 1.0f, 2.0f), 2.0f);
}

BOOST_AUTO_TEST_CASE(algorithm_fract)
{
  BOOST_CHECK_EQUAL(fract(2.75f), 0.75f);
  BOOST_CHECK_EQUAL(fract(1.75f), 0.75f);
  BOOST_CHECK_EQUAL(fract(0.75f), 0.75f);
  BOOST_CHECK_EQUAL(fract(0.0f), 0.0f);
  BOOST_CHECK_EQUAL(fract(-0.0f), 0.0f);
  BOOST_CHECK_EQUAL(fract(-0.75f), 0.25f);
  BOOST_CHECK_EQUAL(fract(-1.75f), 0.25f);
}

BOOST_AUTO_TEST_CASE(algorithm_step)
{
  BOOST_CHECK_EQUAL(step(0.5f, 0.1f), 0.0f);
  BOOST_CHECK_EQUAL(step(0.5f, 0.5f), 1.0f);
  BOOST_CHECK_EQUAL(step(0.5f, 0.9f), 1.0f);
  BOOST_CHECK_EQUAL(step(-0.5f, -0.1f), 1.0f);
  BOOST_CHECK_EQUAL(step(-0.5f, -0.5f), 1.0f);
  BOOST_CHECK_EQUAL(step(-0.5f, -0.9f), 0.0f);
}

BOOST_AUTO_TEST_CASE(algorithm_mix)
{
  BOOST_CHECK_EQUAL(mix(1.0f, 2.0f, 0.0f), 1.0f);
  BOOST_CHECK_EQUAL(mix(1.0f, 2.0f, 0.25f), 1.25f);
  BOOST_CHECK_EQUAL(mix(1.0f, 2.0f, 0.75f), 1.75f);
  BOOST_CHECK_EQUAL(mix(1.0f, 2.0f, 1.0f), 2.0f);
}

BOOST_AUTO_TEST_CASE(algorithm_hamming_weight)
{
  BOOST_CHECK_EQUAL(hamming_weight(0b0000'0000), 0);
  BOOST_CHECK_EQUAL(hamming_weight(0b0000'0001), 1);
  BOOST_CHECK_EQUAL(hamming_weight(0b1000'0000), 1);
  BOOST_CHECK_EQUAL(hamming_weight(0b1000'0001), 2);
  BOOST_CHECK_EQUAL(hamming_weight(0b0100'0010), 2);
  BOOST_CHECK_EQUAL(hamming_weight(0b0101'1010), 4);
  BOOST_CHECK_EQUAL(hamming_weight(0b1010'0101), 4);
  BOOST_CHECK_EQUAL(hamming_weight(0b1111'1111), 8);
}

BOOST_AUTO_TEST_CASE(algorithm_at)
{
  enum class at_test
  {
    cat,
    dog,
    bird
  };
  std::array<int, 3> array{1, 2, 3};

  BOOST_CHECK_EQUAL(at(array, at_test::cat), 1);

  const auto& const_array_ref = array;
  BOOST_CHECK_EQUAL(at(const_array_ref, at_test::dog), 2);

  at(array, at_test::bird) = 42;
  BOOST_CHECK_EQUAL(at(array, at_test::bird), 42);
}
