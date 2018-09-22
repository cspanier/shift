#ifndef SHIFT_CORE_CONTRACT_H
#define SHIFT_CORE_CONTRACT_H

#if __has_cpp_attribute(expects)
#define SHIFT_EXPECTS(e) [[expects:e]]
#define SHIFT_ENSURES(e) [[ensures:e]]
#define SHIFT_ASSERT(e) [[assert:e]]
#else
#include <shift/platform/assert.h>

#define SHIFT_EXPECTS(e)
#define SHIFT_ENSURES(e)
#define SHIFT_ASSERT(e) BOOST_ASSERT(e)
#endif

#endif
