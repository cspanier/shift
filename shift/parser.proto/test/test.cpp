#include "test.hpp"
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>

using namespace shift;
using namespace shift::parser;
using namespace shift::parser::proto;

bool test_parser_and_ast(std::string_view source_code,
                         std::string_view source_name,
                         std::string_view expected_error)
{
  proto::document document;
  return test_parser_and_ast(document, source_code, source_name,
                             expected_error);
}

bool test_parser_and_ast(proto::document& document,
                         std::string_view source_code,
                         std::string_view source_name,
                         std::string_view expected_error)
{
  document = proto::document{};
  auto source = std::make_shared<source_module>(source_code, source_name, 0);
  if (expected_error.empty())
    BOOST_CHECK(document.parse({source}));
  else
    BOOST_CHECK(!document.parse({source}));
  auto errors = document.errors();
  BOOST_CHECK_EQUAL(errors, expected_error);

  return errors == expected_error;
}
