#ifndef SHIFT_CORE_CONTRACT_HPP
#define SHIFT_CORE_CONTRACT_HPP

// C++20 adds support for function level preconditions, postconditions and
// assertions. This header adds macros for an initial transition towards the new
// standard. See https://en.cppreference.com/w/cpp/language/attributes/contract
// for details.

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(expects)
#define SHIFT_EXPECTS(e) [[expects:e]]
#define SHIFT_ENSURES(e) [[ensures:e]]
#define SHIFT_ASSERT(e) [[assert:e]]
#endif
#endif

#if !defined(SHIFT_EXPECTS)
#include <shift/platform/assert.hpp>

#define SHIFT_EXPECTS(e)
#define SHIFT_ENSURES(e)
#define SHIFT_ASSERT(e) BOOST_ASSERT(e)
#endif

#endif
