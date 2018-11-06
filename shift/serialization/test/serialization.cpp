#include <shift/serialization/all.hpp>
#include <shift/serialization/compact/all.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <shift/core/boost_restore_warnings.hpp>
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

using namespace shift::serialization;

template <class InputArchive, class OutputArchive, typename T>
void test(T input)
{
  std::vector<char> buffer;
  {
    OutputArchive output_archive(1);
    output_archive.push(boost::iostreams::back_inserter(buffer));
    output_archive << begin_write{"test", 0};
    output_archive << begin_field{"value"};
    output_archive << input;
    output_archive << end_field{};
    output_archive << end_write{};
  }

  T output;
  {
    InputArchive input_archive(1);
    input_archive.push(
      boost::iostreams::array_source(buffer.data(), buffer.size()));
    input_archive >> begin_read{"test", 0};
    input_archive >> begin_field{"value"};
    input_archive >> output;
    input_archive >> end_field{};
    input_archive >> end_read{};
  }

  BOOST_CHECK_EQUAL(input, output);
}

template <class InputArchive, class OutputArchive>
void test_fundamentals_impl()
{
  test<InputArchive, OutputArchive, bool>(true);
  test<InputArchive, OutputArchive, char>('x');
  test<InputArchive, OutputArchive, char16_t>('x');
  test<InputArchive, OutputArchive, char32_t>('x');
  test<InputArchive, OutputArchive, std::int8_t>(-0x01);
  test<InputArchive, OutputArchive, std::uint8_t>(0x10);
  test<InputArchive, OutputArchive, std::int16_t>(-0x0123);
  test<InputArchive, OutputArchive, std::uint16_t>(0x3210);
  test<InputArchive, OutputArchive, std::int32_t>(-0x01234567);
  test<InputArchive, OutputArchive, std::uint32_t>(0x76543210);
  test<InputArchive, OutputArchive, std::int64_t>(-0x0123456789abcdef);
  test<InputArchive, OutputArchive, std::uint64_t>(0xfedcba9876543210);
  // test<InputArchive, OutputArchive>(std::chrono::system_clock::now());
  test<InputArchive, OutputArchive>(3.14159265f);
  test<InputArchive, OutputArchive>(3.14159265358979);
}

BOOST_AUTO_TEST_CASE(test_fundamentals)
{
  test_fundamentals_impl<compact_input_archive<>, compact_output_archive<>>();
}

template <class InputArchive, class OutputArchive>
void test_string_impl()
{
  test<InputArchive, OutputArchive>(std::string(0x100, ' '));
  test<InputArchive, OutputArchive>(std::string(0x10000, ' '));
}

BOOST_AUTO_TEST_CASE(test_string)
{
  test_string_impl<compact_input_archive<>, compact_output_archive<>>();
}

template <class InputArchive, class OutputArchive>
void test_containers_impl()
{
  test<InputArchive, OutputArchive>(std::array<std::int32_t, 0>{});
  test<InputArchive, OutputArchive>(std::array<std::int32_t, 1>{1});
  test<InputArchive, OutputArchive>(std::array<std::int32_t, 5>{1, 2, 3, 4, 5});

  test<InputArchive, OutputArchive>(std::vector<std::int32_t>{});
  test<InputArchive, OutputArchive>(std::vector<std::int32_t>{1});
  test<InputArchive, OutputArchive>(std::vector<std::int32_t>{1, 2, 3, 4, 5});
}

BOOST_AUTO_TEST_CASE(test_containers)
{
  test_containers_impl<compact_input_archive<>, compact_output_archive<>>();
}
