#ifndef SHIFT_PLATFORM_ASSERT_H
#define SHIFT_PLATFORM_ASSERT_H

#include <shift/core/boost_disable_warnings.h>
#include <boost/assert.hpp>
#include <shift/core/boost_restore_warnings.h>

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
