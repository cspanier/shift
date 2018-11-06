#include <shift/parser/proto/proto.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <iostream>
#include <sstream>
#include "test.hpp"

using namespace shift;
using namespace shift::parser;
using namespace shift::parser::proto;

BOOST_AUTO_TEST_CASE(enum_valids)
{
  proto::document document;

  test_parser_and_ast(
    R"(
enum foo : var_uint_t
{
})",
    "enum_valids_1.pro2");

  while (test_parser_and_ast(document,
                             R"(
enum foo : var_uint_t
{
  a,
  b
})",
                             "enum_valids_2.pro2"))
  {
    const auto& enumerations = document.root().enumerations;
    BOOST_CHECK_EQUAL(enumerations.size(), 1);
    if (enumerations.size() != 1)
      break;
    (void)enumerations.find("foo");
    break;
  }

  while (test_parser_and_ast(document,
                             R"(
enum foo : var_uint_t
{
  a = 2,
  b = 4
})",
                             "enum_valids_3.pro2"))
  {
    break;
  }

  while (test_parser_and_ast(document,
                             R"(
enum foo : var_uint_t
{
  a = 2,
  b
})",
                             "enum_valids_4.pro2"))
  {
    break;
  }

  while (test_parser_and_ast(document,
                             R"(
enum foo : var_uint_t
{
  a,
  b = 4
})",
                             "enum_valids_5.pro2"))
  {
    break;
  }
}

BOOST_AUTO_TEST_CASE(enum_templates)
{
  test_parser_and_ast(
    R"(
enum foo<> : var_uint_t {})",
    "enum_templates_1.pro2",
    R"(enum_templates_1.pro2(2): error P1121: parse error: Expected list of template parameters.
enum foo<> : var_uint_t {}
         ^
)");

  test_parser_and_ast(
    R"(
enum foo<typename T> : T {})",
    "enum_templates_2.pro2");

  test_parser_and_ast(
    R"(
enum foo<var_uint_t N> : var_uint_t { value = N })",
    "enum_templates_3.pro2");

  test_parser_and_ast(
    R"(
enum foo<typename T, T N> : T { value = N })",
    "enum_templates_4.pro2");
}

BOOST_AUTO_TEST_CASE(enum_errors)
{
  test_parser_and_ast(
    R"(enum foo)", "enum_errors_1.pro2",
    R"(enum_errors_1.pro2(1): error P1102: parse error: Expected colon (':').
enum foo
        ^
)");
  test_parser_and_ast(
    R"(enum foo :)", "enum_errors_2.pro2",
    R"(enum_errors_2.pro2(1): error P1120: parse error: Expected type path.
enum foo :
          ^
)");

  test_parser_and_ast(
    R"(
enum foo : var_uint_t
{
  a,
  b,
})",
    "enum_errors_3.pro2",
    R"(enum_errors_3.pro2(5): error P1109: parse error: Expected closing curly bracket ('}').
  b,
   ^
)");

  test_parser_and_ast(
    R"(
enum foo : var_uint_t
{
  a = 1,
  b = 2,
})",
    "enum_errors_4.pro2",
    R"(enum_errors_4.pro2(5): error P1109: parse error: Expected closing curly bracket ('}').
  b = 2,
       ^
)");
}
