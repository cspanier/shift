#include <shift/serialization2/all.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <chrono>
#include <ctime>
#include <string>
#include <cstdio>

namespace std
{
std::ostream& operator<<(std::ostream& stream,
                         std::chrono::system_clock::time_point time_point)
{
  using namespace std::chrono;

  auto time = system_clock::to_time_t(time_point);
  std::string ctime = std::ctime(&time);
  while (ctime.back() == '\n')
    ctime.pop_back();
  stream << ctime << " ("
         << duration_cast<nanoseconds>(time_point.time_since_epoch()).count()
         << "ns)";
  return stream;
}

template <typename T>
std::ostream& operator<<(std::ostream& stream, const std::vector<T> vector)
{
  stream << "{";
  for (const auto& value : vector)
    stream << " " << value;
  stream << " }";
  return stream;
}

template <typename T, std::size_t N>
std::ostream& operator<<(std::ostream& stream, const std::array<T, N> array)
{
  stream << "{";
  for (const auto& value : array)
    stream << " " << value;
  stream << " }";
  return stream;
}
}

using namespace shift::serialization2;

template <typename T>
void test(T&& input)
{
  std::vector<char> buffer;
  {
    boost::iostreams::filtering_ostream stream;
    stream.push(boost::iostreams::back_inserter(buffer));
    compact_output_archive<> output_archive(stream);
    output_archive << begin_write{};
    output_archive << begin_field{};
    output_archive << input;
    output_archive << end_field{};
    output_archive << end_write{};
  }

  T output;
  {
    boost::iostreams::filtering_istream stream;
    stream.push(boost::iostreams::array_source(buffer.data(), buffer.size()));
    compact_input_archive<> input_archive(stream);
    input_archive >> begin_read{};
    input_archive >> begin_field{};
    input_archive >> output;
    input_archive >> end_field{};
    input_archive >> end_read{};
  }

  BOOST_CHECK_EQUAL(input, output);
}

BOOST_AUTO_TEST_CASE(test_fundamentals)
{
  test<bool>(true);
  test<char>('x');
  test<char16_t>('x');
  test<char32_t>('x');
  test<std::int8_t>(-0x01);
  test<std::uint8_t>(0x10);
  test<std::int16_t>(-0x0123);
  test<std::uint16_t>(0x3210);
  test<std::int32_t>(-0x01234567);
  test<std::uint32_t>(0x76543210);
  test<std::int64_t>(-0x0123456789abcdef);
  test<std::uint64_t>(0xfedcba9876543210);
  // test(std::chrono::system_clock::now());
  test(3.14159265f);
  test(3.14159265358979);
}

BOOST_AUTO_TEST_CASE(test_string)
{
  test(std::string(0x100, ' '));
  test(std::string(0x10000, ' '));
}

BOOST_AUTO_TEST_CASE(test_containers)
{
  test(std::array<std::int32_t, 0>{});
  test(std::array<std::int32_t, 1>{1});
  test(std::array<std::int32_t, 5>{1, 2, 3, 4, 5});

  test(std::vector<std::int32_t>{});
  test(std::vector<std::int32_t>{1});
  test(std::vector<std::int32_t>{1, 2, 3, 4, 5});
}
