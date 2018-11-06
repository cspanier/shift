#ifndef SHIFT_CORE_CORE_HPP
#define SHIFT_CORE_CORE_HPP

#include <memory>

#if defined(_MSC_VER) && (_MSC_VER < 1900)
#error Unsupported compiler
#endif

#if defined(__GNUC__)
#define DEPRECATED [[gnu::deprecated]]
#define DEPRECATED_MSG(message) [[gnu::deprecated(#message)]]
#else
#define DEPRECATED [[deprecated]]
#define DEPRECATED_MSG(message) [[deprecated(#message)]]
#endif

#if defined(__GNUC__) || defined(__clang__)
#define SHIFT_EXPORT [[gnu::visibility("default")]]
#define SHIFT_IMPORT
#elif defined(_MSC_VER)
#define SHIFT_EXPORT __declspec(dllexport)
#define SHIFT_IMPORT __declspec(dllimport)
#else
#define SHIFT_EXPORT
#define SHIFT_IMPORT
#endif

#if defined(SHIFT_CORE_SHARED)
#if defined(SHIFT_CORE_EXPORT)
#define SHIFT_CORE_API SHIFT_EXPORT
#else
#define SHIFT_CORE_API SHIFT_IMPORT
#endif
#else
#define SHIFT_CORE_API
#endif

#include <shift/platform/assert.hpp>

/// A separate namespace used only for literal operators, which you may inline
/// using the 'using namespace' directive.
namespace shift::core::literals
{
/// User-defined literal operator used to clearly identify values in kilo bytes.
/// @remarks
///   Note that IEC 80000-13 binary prefixes start with an uppercase letter.
constexpr unsigned long long operator""_KiB(unsigned long long value)
{
  return value * 1024;
}

/// User-defined literal operator used to clearly identify values in kilo bytes.
/// @remarks
///   Note that IEC 80000-13 binary prefixes start with an uppercase letter.
constexpr long double operator""_KiB(long double value)
{
  return value * 1024;
}

/// User-defined literal operator used to clearly identify values in kilo bytes.
/// @remarks
///   Note that IEC 80000-13 binary prefixes start with an uppercase letter.
constexpr unsigned long long operator""_MiB(unsigned long long value)
{
  return value * (1024 * 1024);
}

/// User-defined literal operator used to clearly identify values in kilo bytes.
/// @remarks
///   Note that IEC 80000-13 binary prefixes start with an uppercase letter.
constexpr long double operator""_MiB(long double value)
{
  return value * (1024 * 1024);
}

/// User-defined literal operator used to clearly identify values in kilo bytes.
/// @remarks
///   Note that IEC 80000-13 binary prefixes start with an uppercase letter.
constexpr unsigned long long operator""_GiB(unsigned long long value)
{
  return value * (1024 * 1024 * 1024);
}

/// User-defined literal operator used to clearly identify values in kilo bytes.
/// @remarks
///   Note that IEC 80000-13 binary prefixes start with an uppercase letter.
constexpr long double operator""_GiB(long double value)
{
  return value * (1024 * 1024 * 1024);
}
}

#endif
