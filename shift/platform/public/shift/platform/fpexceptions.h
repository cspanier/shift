#ifndef SHIFT_PLATFORM_FPEXCEPTIONS_H
#define SHIFT_PLATFORM_FPEXCEPTIONS_H

#include <cfloat>

namespace shift::platform
{
#if defined(SHIFT_PLATFORM_LINUX)
#define _EM_ZERODIVIDE 0
#define _EM_INEXACT 0
#define _EM_INVALID 0
#define _EM_OVERFLOW 0
#define _EM_UNDERFLOW 0
#define _EM_DENORMAL 0
#define _MCW_EM 0
#endif

/// This RAII style type enables floating-point exceptions in a block of code.
struct floating_point_exceptions
{
public:
  /// Constructor enabling floating-point exceptions.
  /// @param exceptions
  ///   States which type of floating-point exceptions are to be enable.
  ///   This is a combination of the following bit-flags:
  ///     _EM_ZERODIVIDE : pole error occurred
  ///     _EM_INEXACT    : rounding was necessary to store the result
  ///     _EM_INVALID    : domain error occurred
  ///     _EM_OVERFLOW   : a result was too large to be representable
  ///     _EM_UNDERFLOW  : a result was subnormal with a loss of precision
  ///     _EM_DENORMAL   : an operand has an absolute value that is too small
  ///                      to represent with full precision in the significand
  ///     _MCW_EM        : a combination of all supported exception types.
  /// @param mask
  ///   A bitmask of exceptions to change to the according bit set in the
  ///   exceptions argument. Setting a bit to zero keeps the original state.
  ///   A value of _MCW_EM overwrites all supported exception states.
  floating_point_exceptions(unsigned int exceptions = _MCW_EM,
                            unsigned int mask = _MCW_EM);

  /// Destructor resetting exception behavior to what it was.
  ~floating_point_exceptions();

private:
#if defined(__clang__)
  [[maybe_unused]]
#endif
    unsigned int _old_control_word;
};
}

#endif
