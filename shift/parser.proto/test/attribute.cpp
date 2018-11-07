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

BOOST_AUTO_TEST_CASE(attribute_valids)
{
  proto::document document;

  test_parser_and_ast(
    R"(
[my_attr_1, my_attr_2 = 42, my_attr_3="string with \"quotes\""]
enum foo : var_uint_t
{
  [my_attr_1]
  bar
})",
    "attribute_valids_1.pro2");
}
