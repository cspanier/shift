#ifndef SHIFT_CORE_CORE_H
#define SHIFT_CORE_CORE_H

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

#include <shift/platform/assert.h>

#endif
