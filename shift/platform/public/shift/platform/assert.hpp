#ifndef SHIFT_PLATFORM_ASSERT_HPP
#define SHIFT_PLATFORM_ASSERT_HPP

#include <shift/core/boost_disable_warnings.hpp>
#include <boost/assert.hpp>
#include <shift/core/boost_restore_warnings.hpp>

namespace shift::platform
{
/// Custom assert handler used by Boost assertion macros.
/// @remarks
///   Function could be declared [[noreturn]], but we can't because Boost
///   predeclares the function without.
void assertion_failed_msg(char const* expr, char const* msg,
                          char const* function, char const* file, long line);

/// Custom assert handler used by Boost assertion macros.
void assertion_failed(char const* expr, char const* function, char const* file,
                      long line);
}

#endif
