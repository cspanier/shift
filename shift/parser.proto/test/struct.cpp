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

BOOST_AUTO_TEST_CASE(struct_1)
{
  test_parser_and_ast(
    R"(struct foo :)", "struct_1.pro2",
    R"(struct_1.pro2(1): error P1120: parse error: Expected type path.
struct foo :
            ^
)");
  test_parser_and_ast(
    R"(struct foo :
{
})",
    "struct_2.pro2",
    R"(struct_2.pro2(1): error P1120: parse error: Expected type path.
struct foo :
            ^
)");
  test_parser_and_ast(
    R"(struct foo : {})", "struct_3.pro2",
    R"(struct_3.pro2(1): error P1120: parse error: Expected type path.
struct foo : {}
            ^
)");
}
