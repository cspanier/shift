#include <shift/parser/xml/xml.hpp>
#include <shift/platform/environment.hpp>
#include <iostream>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>

struct init_current_working_path
{
  init_current_working_path() noexcept
  {
    using namespace boost::filesystem;
    boost::system::error_code error;
    current_path(path(shift::platform::environment::executable_path())
                   .parent_path()
                   .parent_path(),
                 error);
    if (error)
      std::terminate();
  }
};

static init_current_working_path init;

using namespace shift::parser::xml;

/// ToDo: Not supported, yet.
// BOOST_AUTO_TEST_CASE(schema_collada_1_4_1)
//{
//  document root;
//  // root.load("../private/collada/XMLSchema.xsd");
//  root.load("../private/collada/collada_schema_1_4_1.xsd");
//
//  printer{std::cout}(root);
//}
