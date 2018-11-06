#include <shift/core/stream_util.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>

using namespace shift::core;

struct my_binary_serializable_type
{
  void serialize(std::ostream& stream) const
  {
    stream << value;
    serialize_ok = true;
  }

  void deserialize(std::istream& stream)
  {
    stream >> value;
    deserialize_ok = true;
  }

  std::string value;
  static bool serialize_ok;
  static bool deserialize_ok;
};

bool my_binary_serializable_type::serialize_ok = false;
bool my_binary_serializable_type::deserialize_ok = false;

BOOST_AUTO_TEST_CASE(streamutil_binary)
{
  {
    std::stringstream ss;
    constexpr std::uint32_t constant = 42;
    std::uint32_t variable = 0;

    ss << binary(constant);
    ss.seekg(0);
    ss >> binary(variable);
    BOOST_CHECK_EQUAL(variable, constant);
    BOOST_CHECK_EQUAL(static_cast<std::size_t>(ss.tellg()), sizeof(variable));
  }

  {
    std::stringstream ss;
    const my_binary_serializable_type constant = {"Hallo_World!"};
    ss << binary(constant);
    BOOST_CHECK(my_binary_serializable_type::serialize_ok);
    BOOST_CHECK_EQUAL(ss.str(), constant.value);

    my_binary_serializable_type variable;
    ss.seekg(0);
    ss >> binary(variable);
    BOOST_CHECK(my_binary_serializable_type::deserialize_ok);
    BOOST_CHECK_EQUAL(variable.value, constant.value);
  }
}
