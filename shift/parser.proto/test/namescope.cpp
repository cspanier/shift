#include <shift/parser/proto/proto.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <iostream>
#include <sstream>
#include "test.h"

using namespace shift;
using namespace shift::parser;
using namespace shift::parser::proto;

BOOST_AUTO_TEST_CASE(namescope_errors)
{
  test_parser_and_ast(
    R"(namescope foo)", "namescope_errors_1.pro2",
    R"(namescope_errors_1.pro2(1): error P1108: parse error: Expected opening curly bracket ('{').
namescope foo
             ^
)");

  test_parser_and_ast(
    R"(namescope {})", "namescope_errors_2.pro2",
    R"(namescope_errors_2.pro2(1): error P1116: parse error: Expected identifier.
namescope {}
          ^
)");
}
