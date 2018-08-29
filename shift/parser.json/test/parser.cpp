#include <shift/parser/json/json.h>
#include <shift/core/stream_util.h>
#include <sstream>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>

using namespace shift;
using namespace shift::parser;

namespace std
{
inline ostream& operator<<(ostream& stream, nullptr_t)
{
  stream << "null";
  return stream;
}
}

BOOST_AUTO_TEST_CASE(parser_empty_document)
{
  json::value root;
  std::istringstream document1("");
  BOOST_CHECK_THROW(document1 >> root, json::parse_error);
}

BOOST_AUTO_TEST_CASE(parser_object)
{
  json::value root;
  std::istringstream document1(R"({})");
  BOOST_CHECK_NO_THROW(document1 >> root);
  std::istringstream document2(R"({"k1" : 1})");
  BOOST_CHECK_NO_THROW(document2 >> root);
  std::istringstream document3(R"({"k1" : 1, "k2" : 2})");
  BOOST_CHECK_NO_THROW(document3 >> root);
}

BOOST_AUTO_TEST_CASE(parser_null)
{
  json::value root;
  std::istringstream document1(R"(null)");
  BOOST_CHECK_NO_THROW(document1 >> root);
  BOOST_CHECK_EQUAL(json::get<std::nullptr_t>(root), nullptr);

  std::istringstream document2(R"({"k1" : null})");
  BOOST_CHECK_NO_THROW(document2 >> root);
  auto& root_object = json::get<json::object>(root);
  BOOST_CHECK_EQUAL(json::get<std::nullptr_t>(root_object, "k1"), nullptr);
}

BOOST_AUTO_TEST_CASE(parser_bool)
{
  json::value root;

  std::istringstream document1(R"(false)");
  BOOST_CHECK_NO_THROW(document1 >> root);
  BOOST_CHECK_EQUAL(json::get<bool>(root), false);

  std::istringstream document2(R"(true)");
  BOOST_CHECK_NO_THROW(document2 >> root);
  BOOST_CHECK_EQUAL(json::get<bool>(root), true);

  std::istringstream document3(R"({"k1" : false, "k2" : true})");
  BOOST_CHECK_NO_THROW(document3 >> root);
  auto& root_object = json::get<json::object>(root);
  BOOST_CHECK_EQUAL(json::get<bool>(root_object, "k1"), false);
  BOOST_CHECK_EQUAL(json::get<bool>(root_object, "k2"), true);
}

BOOST_AUTO_TEST_CASE(parser_numbers)
{
  json::value root;

  std::istringstream document1(R"(1)");
  BOOST_CHECK_NO_THROW(document1 >> root);

  std::istringstream document2(R"(+1)");
  BOOST_CHECK_NO_THROW(document2 >> root);

  std::istringstream document3(R"(-1)");
  BOOST_CHECK_NO_THROW(document3 >> root);

  std::istringstream document4(R"(1.0)");
  BOOST_CHECK_NO_THROW(document4 >> root);

  std::istringstream document5(R"(+1.0)");
  BOOST_CHECK_NO_THROW(document5 >> root);

  std::istringstream document6(R"(-1.0)");
  BOOST_CHECK_NO_THROW(document6 >> root);

  std::istringstream document7(R"(1.0e1)");
  BOOST_CHECK_NO_THROW(document7 >> root);

  std::istringstream document8(R"(1E1)");
  BOOST_CHECK_NO_THROW(document8 >> root);

  std::istringstream document9(R"(-1.0E-10)");
  BOOST_CHECK_NO_THROW(document9 >> root);

  {
    std::istringstream document10(R"({"k1" : 1, "k2" : +1, "k3" : -1})");
    BOOST_CHECK_NO_THROW(document10 >> root);
    auto& root_object = json::get<json::object>(root);
    BOOST_CHECK_EQUAL(json::get<double>(root_object, "k1"), 1);
    BOOST_CHECK_EQUAL(json::get<double>(root_object, "k2"), +1);
    BOOST_CHECK_EQUAL(json::get<double>(root_object, "k3"), -1);
  }

  {
    std::istringstream document11(R"({"k1" : 1.0, "k2" : +1.0, "k3" : -1.0})");
    BOOST_CHECK_NO_THROW(document11 >> root);
    auto& root_object = json::get<json::object>(root);
    BOOST_CHECK_EQUAL(json::get<double>(root_object, "k1"), 1.0);
    BOOST_CHECK_EQUAL(json::get<double>(root_object, "k2"), 1.0);
    BOOST_CHECK_EQUAL(json::get<double>(root_object, "k3"), -1.0);
  }

  {
    std::istringstream document12(
      R"({"k1" : 1.0e1, "k2" : 1E1, "k3" : -1.0E-10})");
    BOOST_CHECK_NO_THROW(document12 >> root);
    auto& root_object = json::get<json::object>(root);
    BOOST_CHECK_EQUAL(json::get<double>(root_object, "k1"), 1.0e1);
    BOOST_CHECK_EQUAL(json::get<double>(root_object, "k2"), 1.0e1);
    BOOST_CHECK_EQUAL(json::get<double>(root_object, "k3"), -1.0e-10);
  }
}

BOOST_AUTO_TEST_CASE(parser_strings)
{
  json::value root;
  {
    std::istringstream document1(R"("")");
    BOOST_CHECK_NO_THROW(document1 >> root);
    BOOST_CHECK_EQUAL(json::get<std::string>(root), "");
  }
  {
    std::istringstream document2(R"("value")");
    BOOST_CHECK_NO_THROW(document2 >> root);
    BOOST_CHECK_EQUAL(json::get<std::string>(root), "value");
  }
  {
    std::istringstream document3(R"({"k1" : ""})");
    BOOST_CHECK_NO_THROW(document3 >> root);
    auto& root_object = json::get<json::object>(root);
    BOOST_CHECK_EQUAL(json::get<std::string>(root_object, "k1"), "");
  }
  {
    std::istringstream document4(R"({"k2" : "value"})");
    BOOST_CHECK_NO_THROW(document4 >> root);
    auto& root_object = json::get<json::object>(root);
    BOOST_CHECK_EQUAL(json::get<std::string>(root_object, "k2"), "value");
  }
  {
    std::istringstream document5(R"({"k3" : "\/"})");
    BOOST_CHECK_NO_THROW(document5 >> root);
    auto& root_object = json::get<json::object>(root);
    BOOST_CHECK_EQUAL(json::get<std::string>(root_object, "k3"), "/");
  }
  {
    std::istringstream document6(R"({"k4" : "\\"})");
    BOOST_CHECK_NO_THROW(document6 >> root);
    auto& root_object = json::get<json::object>(root);
    BOOST_CHECK_EQUAL(json::get<std::string>(root_object, "k4"), "\\");
  }
  {
    std::istringstream document7(R"({"k5" : "\""})");
    BOOST_CHECK_NO_THROW(document7 >> root);
    auto& root_object = json::get<json::object>(root);
    BOOST_CHECK_EQUAL(json::get<std::string>(root_object, "k5"), "\"");
  }
  {
    std::istringstream document8(R"({"k6" : "\b"})");
    BOOST_CHECK_NO_THROW(document8 >> root);
    auto& root_object = json::get<json::object>(root);
    BOOST_CHECK_EQUAL(json::get<std::string>(root_object, "k6"), "\b");
  }
  {
    std::istringstream document9(R"({"k7" : "\f"})");
    BOOST_CHECK_NO_THROW(document9 >> root);
    auto& root_object = json::get<json::object>(root);
    BOOST_CHECK_EQUAL(json::get<std::string>(root_object, "k7"), "\f");
  }
  {
    std::istringstream document10(R"({"k8" : "\n"})");
    BOOST_CHECK_NO_THROW(document10 >> root);
    auto& root_object = json::get<json::object>(root);
    BOOST_CHECK_EQUAL(json::get<std::string>(root_object, "k8"), "\n");
  }
  {
    std::istringstream document11(R"({"k9" : "\r"})");
    BOOST_CHECK_NO_THROW(document11 >> root);
    auto& root_object = json::get<json::object>(root);
    BOOST_CHECK_EQUAL(json::get<std::string>(root_object, "k9"), "\r");
  }
  {
    std::istringstream document12(R"({"k10" : "\t"})");
    BOOST_CHECK_NO_THROW(document12 >> root);
    auto& root_object = json::get<json::object>(root);
    BOOST_CHECK_EQUAL(json::get<std::string>(root_object, "k10"), "\t");
  }
  {
    std::istringstream document13(R"({"k11" : "\u20"})");
    BOOST_CHECK_NO_THROW(document13 >> root);
    auto& root_object = json::get<json::object>(root);
    BOOST_CHECK_EQUAL(json::get<std::string>(root_object, "k11"), " ");
  }
}

BOOST_AUTO_TEST_CASE(parser_arrays)
{
  json::value root;
  std::istringstream document1(R"([])");
  BOOST_CHECK_NO_THROW(document1 >> root);

  std::istringstream document2(R"([[]])");
  BOOST_CHECK_NO_THROW(document2 >> root);

  {
    std::istringstream document3(
      R"({"k1" : [], "k2" : [1, "foo", true, null]})");
    BOOST_CHECK_NO_THROW(document3 >> root);
    auto& root_object = json::get<json::object>(root);

    const auto& array1 = json::get<json::array>(root_object, "k1");
    BOOST_CHECK(array1.empty());

    const auto& array2 = json::get<json::array>(root_object, "k2");
    BOOST_CHECK_EQUAL(array2.size(), 4);
    BOOST_CHECK_EQUAL(json::get<double>(array2[0]), 1);
    BOOST_CHECK_EQUAL(json::get<std::string>(array2[1]), "foo");
    BOOST_CHECK_EQUAL(json::get<bool>(array2[2]), true);
    BOOST_CHECK_EQUAL(json::get<std::nullptr_t>(array2[3]), nullptr);
  }
}

#if defined(SHIFT_PLATFORM_WINDOWS)
#define br "\r\n"
#else
#define br "\n"
#endif
#define br2 br br

BOOST_AUTO_TEST_CASE(parser_print)
{
  // Don't use raw string literals here to normalize line breaks.
  // clang-format off
  std::string source =
    "{" br
    "  \"k1\" : []," br
    "  \"k2\" : [" br
    "    1," br
    "    \"foo\"," br
    "    true," br
    "    null" br
    "  ]," br
    "  \"k3\" : {}," br
    "  \"k4\" : {" br
    "    \"j1\" : 5" br
    "  }," br
    "  \"k5\" : [" br
    "    {" br
    "      \"name\" : \"Hans\"" br
    "    }," br
    "    {" br
    "      \"name\" : \"Peter\"" br
    "    }" br
    "  ]" br
    "}";
  // clang-format on
  json::value root;
  std::istringstream document1(source);
  BOOST_CHECK_NO_THROW(document1 >> root);
  std::stringstream destination;
  destination << core::indent_width(2) << root << std::flush;
  BOOST_CHECK_EQUAL(destination.str(), source);
}

BOOST_AUTO_TEST_CASE(parser_comments)
{
  std::istringstream document1(R"(
# outer comment
{
  # inner comment
  "a" : 1,
  "b" : 2, # comment at end of line
  # comment spanning
  # multiple lines
  "c" : 3,
  "d" : [ 1, 2, # comment in array
    3 ],
  "e" : "this is #no-comment"
})");
  json::value root;
  BOOST_CHECK_NO_THROW(document1 >> root);
}
