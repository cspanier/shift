// Microsoft Visual C++ generates tons of warnings with warning level 4 in
// external Boost libraries. This header disables most of them.
//
// Use this header in conjunction with boost_restore_warnings.h:
//
// #include <shift/core/boost_disable_warnings.h>
// #include <boost/...>
// #include <shift/core/boost_restore_warnings.h>
#if defined(_MSC_VER)
#include <boost/version.hpp>
#if (BOOST_VERSION <= 106800)
#pragma warning(push)

// C4245: 'initializing': conversion from '<type>' to '<type>', signed/unsigned
// mismatch
#pragma warning(disable : 4245)

// C4389: '==': signed/unsigned mismatch
#pragma warning(disable : 4389)

// C4459: declaration of '<var>' hides global declaration.
#pragma warning(disable : 4459)

// C6001: Using uninitialized memory '<var>'.
#pragma warning(disable : 6001)

// C6031: Return value ignored: '<function>'.
#pragma warning(disable : 6031)

// C6246: Local declaration of 'e' hides declaration of the same name in outer
// scope.
#pragma warning(disable : 6246)

// C6255: _alloca indicates failure by raising a stack overflow exception.
// Consider using _malloca instead.
#pragma warning(disable : 6255)

// C6258: Using TerminateThread does not allow proper thread clean up.
#pragma warning(disable : 6258)

// C6323: Use of arithmetic operator on Boolean type(s).
#pragma warning(disable : 6323)

// C6326: Potential comparison of a constant with another constant.
#pragma warning(disable : 6326)

// C6387: '<var>' could be '<value>':  this does not adhere to the specification
// for the function '<WinAPI-function>'.
#pragma warning(disable : 6387)

// C28251: Inconsistent annotation for '<function>': this instance has no
// annotations.
#pragma warning(disable : 28251)
#endif
#endif
