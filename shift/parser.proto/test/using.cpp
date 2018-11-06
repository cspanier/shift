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

BOOST_AUTO_TEST_CASE(using_errors)
{
  test_parser_and_ast(
    R"(using)", "using_errors_1.pro2",
    R"(using_errors_1.pro2(1): error P1100: parse error: Expected whitespace.
using
     ^
)");
  test_parser_and_ast(
    R"(using;)", "using_errors_2.pro2",
    R"(using_errors_2.pro2(1): error P1100: parse error: Expected whitespace.
using;
     ^
)");
  test_parser_and_ast(
    R"(using )", "using_errors_3.pro2",
    R"(using_errors_3.pro2(1): error P1116: parse error: Expected identifier.
using 
      ^
)");
  test_parser_and_ast(
    R"(using ;)", "using_errors_4.pro2",
    R"(using_errors_4.pro2(1): error P1116: parse error: Expected identifier.
using ;
      ^
)");
  test_parser_and_ast(
    R"(using foo)", "using_errors_5.pro2",
    R"(using_errors_5.pro2(1): error P1105: parse error: Expected equality sign ('=').
using foo
         ^
)");
  test_parser_and_ast(
    R"(using foo;)", "using_errors_6.pro2",
    R"(using_errors_6.pro2(1): error P1105: parse error: Expected equality sign ('=').
using foo;
         ^
)");
  test_parser_and_ast(
    R"(using foo =)", "using_errors_7.pro2",
    R"(using_errors_7.pro2(1): error P1120: parse error: Expected type path.
using foo =
           ^
)");
  test_parser_and_ast(
    R"(using foo =;)", "using_errors_8.pro2",
    R"(using_errors_8.pro2(1): error P1120: parse error: Expected type path.
using foo =;
           ^
)");
}
