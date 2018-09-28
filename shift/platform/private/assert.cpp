#include <shift/core/boost_disable_warnings.h>
#include <boost/stacktrace.hpp>
#include <shift/core/boost_restore_warnings.h>
// BOOST_ENABLE_ASSERT_DEBUG_HANDLER is defined for the whole project
#include <stdexcept>
#include <iostream>

namespace boost
{
void assertion_failed_msg(char const* expr, char const* msg,
                          char const* function, char const* /*file*/,
                          long /*line*/)
{
  std::cerr << "Expression '" << expr << "' is false in function '" << function
            << "': " << (msg != nullptr ? msg : "<...>") << ".\n";
  std::abort();
}

void assertion_failed(char const* expr, char const* function, char const* file,
                      long line)
{
  ::boost::assertion_failed_msg(expr, nullptr, function, file, line);
}
}
