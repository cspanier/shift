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

// C4005: '<macro-name>': macro redefinition
#pragma warning(disable : 4005)

// C4245: 'initializing': conversion from '<type>' to '<type>', signed/unsigned
// mismatch
#pragma warning(disable : 4245)

// C4389: '==': signed/unsigned mismatch
#pragma warning(disable : 4389)

// C4459: declaration of '<var>' hides global declaration.
#pragma warning(disable : 4459)

// C4706: assignment within conditional expression
#pragma warning(disable : 4706)

// C4714: function '<symbol>' marked as __forceinline not inlined
#pragma warning(disable : 4714)

// C4996: 'std::allocator<void>': warning STL4009: std::allocator<void> is
// deprecated in C++17. You can define
// _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING or
// _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS to acknowledge that you have received
// this warning.
// C4996: 'std::fpos<_Mbstatet>::seekpos': warning STL4019: The member
// std::fpos::seekpos() is non-Standard, and is preserved only for compatibility
// with workarounds for old versions of Visual C++. It will be removed in a
// future release, and in this release always returns 0. Please use
// standards-conforming mechanisms to manipulate fpos, such as conversions to
// and from streamoff, or an integral type, instead. If you are receiving this
// message while compiling Boost.IOStreams, a fix has been submitted upstream to
// make Boost use standards-conforming mechanisms, as it does for other
// compilers. You can define _SILENCE_FPOS_SEEKPOS_DEPRECATION_WARNING to
// acknowledge that you have received this warning, or define
// _REMOVE_FPOS_SEEKPOS to remove std::fpos::seekpos entirely.
#pragma warning(disable : 4996)

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

// C6285: (<non-zero constant> || <non-zero constant>) is always a non-zero
// constant.  Did you intend to use the bitwise-and operator?
#pragma warning(disable : 6285)

// C6306: Incorrect call to '<function>': consider using '<function>' which
// accepts a va_list as an argument.
#pragma warning(disable : 6306)

// C6323: Use of arithmetic operator on Boolean type(s).
#pragma warning(disable : 6323)

// C6326: Potential comparison of a constant with another constant.
#pragma warning(disable : 6326)

// C6385: Reading invalid data from '<var':  the readable size is '<n>' bytes,
// but '<m>' bytes may be read.
#pragma warning(disable : 6385)

// C6387: '<var>' could be '<value>':  this does not adhere to the specification
// for the function '<WinAPI-function>'.
#pragma warning(disable : 6387)

// C6388: '<var>' might not be '<value>':  this does not adhere to the
// specification for the function '<function>'.
#pragma warning(disable : 6388)

// C28251: Inconsistent annotation for '<function>': this instance has no
// annotations.
#pragma warning(disable : 28251)
#endif
#endif
