// This file is heavily based on "half - IEEE 754-based half-precision floating
// point library" version 1.12.0 by Christian Rau.
//
// The original file was
//
// Copyright (c) 2012-2017 Christian Rau <rauy@users.sourceforge.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef SHIFT_MATH_HALF_HPP
#define SHIFT_MATH_HALF_HPP

#include <climits>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <utility>
#include <algorithm>
#include <iostream>
#include <limits>
#include <array>
#include <type_traits>
#include <functional>

/// Value signaling overflow.
/// In correspondence with `HUGE_VAL[F|L]` from `<cmath>` this symbol expands to
/// a positive value signaling the overflow of an operation, in particular it
/// just evaluates to positive infinity.
#define HUGE_VALH std::numeric_limits<::shift::math::half>::infinity()

#ifndef FP_ILOGB0
#define FP_ILOGB0 INT_MIN
#endif
#ifndef FP_ILOGBNAN
#define FP_ILOGBNAN INT_MAX
#endif
#ifndef FP_SUBNORMAL
#define FP_SUBNORMAL 0
#endif
#ifndef FP_ZERO
#define FP_ZERO 1
#endif
#ifndef FP_NAN
#define FP_NAN 2
#endif
#ifndef FP_INFINITE
#define FP_INFINITE 3
#endif
#ifndef FP_NORMAL
#define FP_NORMAL 4
#endif

namespace shift::math
{
class half;

struct half_config
{
  /// Rounding mode to use.
  /// This specifies the rounding mode used for all conversions between
  /// [half](\ref shift::math::half)s and `float`s as well as for the
  /// half_cast() if not specifying a rounding mode explicitly. It can be
  /// changed to one of the standard rounding modes using their respective
  /// constants of `std::float_round_style`:
  ///
  /// `std::float_round_style`         | rounding
  /// ---------------------------------|-------------------------
  /// `std::round_indeterminate`       | fastest (default)
  /// `std::round_toward_zero`         | toward zero
  /// `std::round_to_nearest`          | to nearest
  /// `std::round_toward_infinity`     | toward positive infinity
  /// `std::round_toward_neg_infinity` | toward negative infinity
  ///
  /// By default this is set to `std::round_indeterminate`, which uses
  /// truncation (round toward zero, but with overflows set to infinity) and is
  /// the fastest rounding mode possible. It can be set to
  /// `std::numeric_limits<float>::round_style` to synchronize the rounding mode
  /// with that of the underlying single-precision implementation.
  static constexpr std::float_round_style round_style =
    std::round_indeterminate;

  /// Tie-breaking behaviour for round to nearest.
  /// This specifies if ties in round to nearest should be resolved by rounding
  /// to the nearest even value. By default this is defined to false resulting
  /// in the faster but slightly more biased behaviour of rounding away from
  /// zero in half-way cases (and thus equal to the round() function), but can
  /// be changed to true if more IEEE-conformant behaviour is needed.
  static constexpr bool round_ties_to_even = false;
};

namespace literals
{
  half operator""_h(long double);
}

/// @internal
/// @brief Implementation details.
namespace detail
{
  /// Helper for tag dispatching.
  template <bool B>
  struct bool_type : std::integral_constant<bool, B>
  {
  };
  using std::false_type;
  using std::true_type;

  /// Tag type for binary construction.
  struct binary_t
  {
  };

  /// Tag for binary construction.
  constexpr binary_t binary = binary_t();

  /// Temporary half-precision expression.
  /// This class represents a half-precision expression which just stores a
  /// single-precision value internally.
  struct expr
  {
    /// Conversion constructor.
    /// @param f single-precision value to convert
    explicit constexpr expr(float f) noexcept : _value(f)
    {
    }

    /// Conversion to single-precision.
    /// @return single precision value representing expression value
    constexpr operator float() const noexcept
    {
      return _value;
    }

  private:
    /// Internal expression value stored in single-precision.
    float _value;
  };

  /// SFINAE helper for generic half-precision functions.
  /// This class template has to be specialized for each valid combination of
  /// argument types to provide a corresponding `type` member equivalent to \a
  /// T.
  /// @tparam T type to return
  template <typename T, typename, typename = void, typename = void>
  struct enable
  {
  };

  template <typename T>
  struct enable<T, half, void, void>
  {
    using type = T;
  };

  template <typename T>
  struct enable<T, expr, void, void>
  {
    using type = T;
  };

  template <typename T>
  struct enable<T, half, half, void>
  {
    using type = T;
  };

  template <typename T>
  struct enable<T, half, expr, void>
  {
    using type = T;
  };

  template <typename T>
  struct enable<T, expr, half, void>
  {
    using type = T;
  };

  template <typename T>
  struct enable<T, expr, expr, void>
  {
    using type = T;
  };

  template <typename T>
  struct enable<T, half, half, half>
  {
    using type = T;
  };

  template <typename T>
  struct enable<T, half, half, expr>
  {
    using type = T;
  };

  template <typename T>
  struct enable<T, half, expr, half>
  {
    using type = T;
  };

  template <typename T>
  struct enable<T, half, expr, expr>
  {
    using type = T;
  };

  template <typename T>
  struct enable<T, expr, half, half>
  {
    using type = T;
  };

  template <typename T>
  struct enable<T, expr, half, expr>
  {
    using type = T;
  };

  template <typename T>
  struct enable<T, expr, expr, half>
  {
    using type = T;
  };

  template <typename T>
  struct enable<T, expr, expr, expr>
  {
    using type = T;
  };

  template <typename T, typename U, typename V = void, typename W = void>
  using enable_t = typename enable<T, U, V, W>::type;

  /// Return type for specialized generic 2-argument half-precision functions.
  /// This class template has to be specialized for each valid combination of
  /// argument types to provide a corresponding `type` member denoting the
  /// appropriate return type.
  /// @tparam T first argument type
  /// @tparam U first argument type
  template <typename T, typename U>
  struct result : enable<expr, T, U>
  {
  };

  template <>
  struct result<half, half>
  {
    using type = half;
  };

  /// @name Conversion
  /// @{
  constexpr auto half_mantissa_table_gen()
  {
    std::array<std::uint32_t, 2048> mantissa_table{};

    mantissa_table[0] = 0;
    for (std::uint32_t i = 1; i <= 1023; ++i)
    {
      std::uint32_t mantissa = i << 13;
      std::uint32_t exponent = 0;
      // While not normalized
      while ((mantissa & (1 << 23)) == 0)
      {
        ++exponent;
        mantissa <<= 1;
      }
      // Clear leading 1 bit
      mantissa &= ~0x00800000U;
      // Adjust bias.
      exponent = 113 - exponent;

      mantissa_table[i] = mantissa | (exponent << 23);
    }
    for (std::uint32_t i = 1024; i < mantissa_table.size(); ++i)
      mantissa_table[i] = 0x38000000 + ((i - 1024) << 13);

    return mantissa_table;
  }

  constexpr auto half_exponent_table_gen()
  {
    std::array<std::uint32_t, 64> exponent_table{};

    exponent_table[0] = 0;
    for (std::uint32_t i = 1; i <= 30; ++i)
      exponent_table[i] = i << 23;
    exponent_table[31] = 0x47800000;
    exponent_table[32] = 0x80000000;
    for (std::uint32_t i = 33; i <= 62; ++i)
      exponent_table[i] = 0x80000000 + ((i - 32) << 23);
    exponent_table[63] = 0xC7800000;

    return exponent_table;
  }

  constexpr auto half_offset_table_gen()
  {
    std::array<std::uint16_t, 64> offset_table{};

    for (std::uint32_t i = 0; i < offset_table.size(); ++i)
      offset_table[i] = (i == 0 || i == 32) ? 0 : 1024;

    return offset_table;
  }

  constexpr auto half_base_table_gen()
  {
    std::array<std::uint16_t, 512> base_table{};

    for (std::uint32_t i = 0; i < base_table.size() / 2; ++i)
    {
      int e = static_cast<int>(i) - 127;
      if (e < -24)
      {
        // Very small numbers map to zero
        base_table[i | 0x000] = 0x0000;
        base_table[i | 0x100] = 0x8000;
      }
      else if (e < -14)
      {
        // Small numbers map to denorms
        base_table[i | 0x000] = (0x0400 >> (-e - 14));
        base_table[i | 0x100] = (0x0400 >> (-e - 14)) | 0x8000;
      }
      else if (e <= 15)
      {
        // Normal numbers just lose precision
        base_table[i | 0x000] = static_cast<std::uint16_t>((e + 15) << 10);
        base_table[i | 0x100] =
          static_cast<std::uint16_t>((e + 15) << 10) | 0x8000;
      }
      else if (e < 128)
      {
        // Large numbers map to Infinity
        base_table[i | 0x000] = 0x7C00;
        base_table[i | 0x100] = 0xFC00;
      }
      else
      {
        // Infinity and NaN's stay Infinity and NaN's
        base_table[i | 0x000] = 0x7C00;
        base_table[i | 0x100] = 0xFC00;
      }
    }

    return base_table;
  }

  constexpr auto half_shift_table_gen()
  {
    std::array<std::uint8_t, 512> shift_table{};

    for (std::uint32_t i = 0; i < shift_table.size() / 2; ++i)
    {
      int e = static_cast<int>(i) - 127;
      if (e < -24)
      {
        // Very small numbers map to zero
        shift_table[i | 0x000] = 24;
        shift_table[i | 0x100] = 24;
      }
      else if (e < -14)
      {
        // Small numbers map to denorms
        shift_table[i | 0x000] = static_cast<std::uint8_t>(-e - 1);
        shift_table[i | 0x100] = static_cast<std::uint8_t>(-e - 1);
      }
      else if (e <= 15)
      {
        // Normal numbers just lose precision
        shift_table[i | 0x000] = 13;
        shift_table[i | 0x100] = 13;
      }
      else if (e < 128)
      {
        // Large numbers map to Infinity
        shift_table[i | 0x000] = 24;
        shift_table[i | 0x100] = 24;
      }
      else
      {
        // Infinity and NaN's stay Infinity and NaN's
        shift_table[i | 0x000] = 13;
        shift_table[i | 0x100] = 13;
      }
    }

    return shift_table;
  }

  /// Convert IEEE single-precision to half-precision.
  /// Credit for this goes to [Jeroen van der
  /// Zijp](ftp://ftp.fox-toolkit.org/pub/fasthalffloatconversion.pdf).
  /// @tparam R rounding mode to use, `std::round_indeterminate` for fastest
  /// rounding
  /// @param value single-precision value
  /// @return binary representation of half-precision value
  template <std::float_round_style R>
  std::uint16_t float2half_impl(float value, true_type)
  {
    std::uint32_t bits{};
    // *reinterpret_cast<std::uint32_t*>(&value) violates strict aliasing!
    static_assert(sizeof(value) == sizeof(bits));
    std::memcpy(&bits, &value, sizeof(value));

    static constexpr auto base_table = half_base_table_gen();
    static constexpr auto shift_table = half_shift_table_gen();

    std::uint16_t hbits =
      base_table[bits >> 23] +
      static_cast<std::uint16_t>((bits & 0x7FFFFF) >> shift_table[bits >> 23]);
    if constexpr (R == std::round_to_nearest)
    {
      if constexpr (half_config::round_ties_to_even)
      {
        hbits +=
          (((bits & 0x7FFFFF) >> (shift_table[bits >> 23] - 1)) |
           (((bits >> 23) & 0xFF) == 102)) &
          ((hbits & 0x7C00) != 0x7C00) &
          (((((static_cast<std::uint32_t>(1) << (shift_table[bits >> 23] - 1)) -
              1) &
             bits) != 0) |
           hbits);
      }
      else
      {
        hbits += (((bits & 0x7FFFFF) >> (shift_table[bits >> 23] - 1)) |
                  (((bits >> 23) & 0xFF) == 102)) &
                 ((hbits & 0x7C00) != 0x7C00);
      }
    }
    else if constexpr (R == std::round_toward_zero)
      hbits -= ((hbits & 0x7FFF) == 0x7C00) & ~shift_table[bits >> 23];
    else if constexpr (R == std::round_toward_infinity)
    {
      hbits +=
        ((((bits & 0x7FFFFF &
            ((static_cast<std::uint32_t>(1) << (shift_table[bits >> 23])) -
             1)) != 0) |
          (((bits >> 23) <= 102) & ((bits >> 23) != 0))) &
         (hbits < 0x7C00)) -
        ((hbits == 0xFC00) & ((bits >> 23) != 511));
    }
    else if constexpr (R == std::round_toward_neg_infinity)
    {
      hbits +=
        ((((bits & 0x7FFFFF &
            ((static_cast<std::uint32_t>(1) << (shift_table[bits >> 23])) -
             1)) != 0) |
          (((bits >> 23) <= 358) & ((bits >> 23) != 256))) &
         (hbits < 0xFC00) & (hbits >> 15)) -
        ((hbits == 0x7C00) & ((bits >> 23) != 255));
    }
    return hbits;
  }

  /// Convert IEEE double-precision to half-precision.
  /// @tparam R rounding mode to use, `std::round_indeterminate` for fastest
  /// rounding
  /// @param value double-precision value
  /// @return binary representation of half-precision value
  template <std::float_round_style R>
  std::uint16_t float2half_impl(double value, true_type)
  {
    std::uint64_t bits{};
    // *reinterpret_cast<std::uint64_t*>(&value) violates strict aliasing!
    static_assert(sizeof(value) == sizeof(bits));
    std::memcpy(&bits, &value, sizeof(value));
    std::uint32_t hi = bits >> 32, lo = bits & 0xFFFFFFFF;
    std::uint16_t hbits = (hi >> 16) & 0x8000;
    hi &= 0x7FFFFFFF;
    int exp = hi >> 20;
    if (exp == 2047)
    {
      return hbits | 0x7C00 |
             (0x3FF & -(((bits & 0xFFFFFFFFFFFFF) != 0) ? 1 : 0));
    }
    if (exp > 1038)
    {
      if constexpr (R == std::round_toward_infinity)
        return hbits | (0x7C00 - (hbits >> 15));
      else if constexpr (R == std::round_toward_neg_infinity)
        return hbits | (0x7BFF + (hbits >> 15));
      else
        return hbits | (0x7BFF + ((R != std::round_toward_zero) ? 1 : 0));
    }
    int g, s = lo != 0;
    if (exp > 1008)
    {
      g = (hi >> 9) & 1;
      s |= (hi & 0x1FF) != 0;
      hbits |=
        (static_cast<unsigned int>(exp - 1008) << 10) | ((hi >> 10) & 0x3FF);
    }
    else if (exp > 997)
    {
      int i = 1018 - exp;
      hi = (hi & 0xFFFFF) | 0x100000;
      g = (hi >> i) & 1;
      s |= (hi & ((1L << i) - 1)) != 0;
      hbits |= hi >> (i + 1);
    }
    else
    {
      g = 0;
      s |= hi != 0;
    }
    if constexpr (R == std::round_to_nearest)
    {
      if constexpr (half_config::round_ties_to_even)
        hbits += g & (s | hbits);
      else
        hbits += g;
    }
    else if constexpr (R == std::round_toward_infinity)
      hbits += ~(hbits >> 15) & (s | g);
    else if constexpr (R == std::round_toward_neg_infinity)
      hbits += (hbits >> 15) & (g | s);
    return hbits;
  }

  /// Convert floating point to half-precision.
  /// @tparam R rounding mode to use, `std::round_indeterminate` for fastest
  /// rounding
  /// @tparam T source type (builtin floating point type)
  /// @param value floating point value
  /// @return binary representation of half-precision value
  template <std::float_round_style R, typename T>
  std::uint16_t float2half(T value)
  {
    return float2half_impl<R>(value,
                              bool_type<std::numeric_limits<T>::is_iec559>{});
  }

  /// Convert integer to half-precision floating point.
  /// @tparam R rounding mode to use, `std::round_indeterminate` for fastest
  /// rounding
  /// @tparam S `true` if value negative, `false` else
  /// @tparam T type to convert (builtin integer type)
  /// @param value non-negative integral value
  /// @return binary representation of half-precision value
  template <std::float_round_style R, bool S, typename T>
  std::uint16_t int2half_impl(T value)
  {
    static_assert(std::is_integral_v<T>,
                  "int to half conversion only supports builtin integer types");
    if (S)
      value = -value;
    std::uint16_t bits = S << 15;
    if (value > 0xFFFF)
    {
      if constexpr (R == std::round_toward_infinity)
        bits |= 0x7C00 - S;
      else if constexpr (R == std::round_toward_neg_infinity)
        bits |= 0x7BFF + S;
      else
        bits |= 0x7BFF + (R != std::round_toward_zero);
    }
    else if (value)
    {
      unsigned int m = value, exp = 24;
      for (; m < 0x400; m <<= 1, --exp)
        ;
      for (; m > 0x7FF; m >>= 1, ++exp)
        ;
      bits |= (exp << 10) + m;
      if (exp > 24)
      {
        if constexpr (R == std::round_to_nearest)
        {
          if constexpr (half_config::round_ties_to_even)
            bits += (value >> (exp - 25)) & 1 &
                    (((((1 << (exp - 25)) - 1) & value) != 0) | bits);
          else
            bits += (value >> (exp - 25)) & 1;
        }
        else if constexpr (R == std::round_toward_infinity)
          bits += ((value & ((1 << (exp - 24)) - 1)) != 0) & !S;
        else if constexpr (R == std::round_toward_neg_infinity)
          bits += ((value & ((1 << (exp - 24)) - 1)) != 0) & S;
      }
    }
    return bits;
  }

  /// Convert integer to half-precision floating point.
  /// @tparam R rounding mode to use, `std::round_indeterminate` for fastest
  /// rounding
  /// @tparam T type to convert (builtin integer type)
  /// @param value integral value
  /// @return binary representation of half-precision value
  template <std::float_round_style R, typename T>
  std::uint16_t int2half(T value)
  {
    return (value < 0) ? int2half_impl<R, true>(value)
                       : int2half_impl<R, false>(value);
  }

  /// Convert half-precision to IEEE single-precision.
  /// Credit for this goes to [Jeroen van der
  /// Zijp](ftp://ftp.fox-toolkit.org/pub/fasthalffloatconversion.pdf).
  /// @param value binary representation of half-precision value
  /// @return single-precision value
  inline float half2float_impl(std::uint16_t value, float, true_type)
  {
    static constexpr auto mantissa_table = half_mantissa_table_gen();
    static constexpr auto exponent_table = half_exponent_table_gen();
    static constexpr auto offset_table = half_offset_table_gen();
    std::uint32_t bits =
      mantissa_table[offset_table[value >> 10] + (value & 0x3FF)] +
      exponent_table[value >> 10];
    float out;
    static_assert(sizeof(bits) == sizeof(out));
    std::memcpy(&out, &bits, sizeof(bits));
    return out;
  }

  /// Convert half-precision to IEEE double-precision.
  /// @param value binary representation of half-precision value
  /// @return double-precision value
  inline double half2float_impl(std::uint16_t value, double, true_type)
  {
    auto hi = static_cast<std::uint32_t>(value & 0x8000) << 16;
    int abs = value & 0x7FFF;
    if (abs)
    {
      hi |= 0x3F000000u << ((abs >= 0x7C00) ? 1 : 0);
      for (; abs < 0x400; abs <<= 1)
        hi -= 0x100000;
      hi += static_cast<std::uint32_t>(abs) << 10;
    }
    auto bits = static_cast<std::uint64_t>(hi) << 32;
    // *reinterpret_cast<double*>(&bits) violating strict aliasing!
    double out;
    static_assert(sizeof(bits) == sizeof(out));
    std::memcpy(&out, &bits, sizeof(bits));
    return out;
  }

  /// Convert half-precision to floating point.
  /// @tparam T type to convert to (builtin integer type)
  /// @param value binary representation of half-precision value
  /// @return floating point value
  template <typename T>
  T half2float(std::uint16_t value)
  {
    return half2float_impl(value, T{},
                           bool_type<std::numeric_limits<T>::is_iec559>());
  }

  /// Convert half-precision floating point to integer.
  /// @tparam R rounding mode to use, `std::round_indeterminate` for fastest
  /// rounding
  /// @tparam E `true` for round to even, `false` for round away from zero
  /// @tparam T type to convert to (buitlin integer type with at least 16 bits
  /// precision, excluding any implicit sign bits)
  /// @param value binary representation of half-precision value
  /// @return integral value
  template <std::float_round_style R, bool E, typename T>
  T half2int_impl(std::uint16_t value)
  {
    static_assert(std::is_integral_v<T>,
                  "half to int conversion only supports builtin integer types");
    unsigned int e = value & 0x7FFF;
    if (e >= 0x7C00)
      return (value & 0x8000) ? std::numeric_limits<T>::min()
                              : std::numeric_limits<T>::max();
    if (e < 0x3800)
    {
      if constexpr (R == std::round_toward_infinity)
        return T(~(value >> 15) & (e != 0));
      else if constexpr (R == std::round_toward_neg_infinity)
        return -T(value > 0x8000);
      return T{};
    }
    unsigned int m = (value & 0x3FF) | 0x400;
    e >>= 10;
    if (e < 25)
    {
      if constexpr (R == std::round_to_nearest)
        m += (1 << (24 - e)) - (~(m >> (25 - e)) & E);
      else if constexpr (R == std::round_toward_infinity)
        m += ((value >> 15) - 1) & ((1 << (25 - e)) - 1U);
      else if constexpr (R == std::round_toward_neg_infinity)
        m += -(value >> 15) & ((1 << (25 - e)) - 1U);
      m >>= 25 - e;
    }
    else
      m <<= e - 25;
    return (value & 0x8000) ? -static_cast<T>(m) : static_cast<T>(m);
  }

  /// Convert half-precision floating point to integer.
  /// @tparam R rounding mode to use, `std::round_indeterminate` for fastest
  /// rounding
  /// @tparam T type to convert to (buitlin integer type with at least 16 bits
  /// precision, excluding any implicit sign bits)
  /// @param value binary representation of half-precision value
  /// @return integral value
  template <std::float_round_style R, typename T>
  T half2int(std::uint16_t value)
  {
    return half2int_impl<R, half_config::round_ties_to_even, T>(value);
  }

  /// Convert half-precision floating point to integer using
  /// round-to-nearest-away-from-zero.
  /// @tparam T type to convert to (buitlin integer type with at least 16 bits
  /// precision, excluding any implicit sign bits)
  /// @param value binary representation of half-precision value
  /// @return integral value
  template <typename T>
  T half2int_up(std::uint16_t value)
  {
    return half2int_impl<std::round_to_nearest, 0, T>(value);
  }

  /// Round half-precision number to nearest integer value.
  /// @tparam R rounding mode to use, `std::round_indeterminate` for fastest
  /// rounding
  /// @tparam E `true` for round to even, `false` for round away from zero
  /// @param value binary representation of half-precision value
  /// @return half-precision bits for nearest integral value
  template <std::float_round_style R, bool E>
  std::uint16_t round_half_impl(std::uint16_t value)
  {
    unsigned int e = value & 0x7FFF;
    std::uint16_t result = value;
    if (e < 0x3C00)
    {
      result &= 0x8000;
      if constexpr (R == std::round_to_nearest)
        result |= 0x3C00U & -(e >= (0x3800 + E));
      else if constexpr (R == std::round_toward_infinity)
        result |= 0x3C00U & -(~(value >> 15) & (e != 0));
      else if constexpr (R == std::round_toward_neg_infinity)
        result |= 0x3C00U & -(value > 0x8000);
    }
    else if (e < 0x6400)
    {
      e = 25 - (e >> 10);
      unsigned int mask = (1 << e) - 1;
      if constexpr (R == std::round_to_nearest)
        result += (1 << (e - 1)) - (~(result >> e) & E);
      else if constexpr (R == std::round_toward_infinity)
        result += mask & ((value >> 15) - 1);
      else if constexpr (R == std::round_toward_neg_infinity)
        result += mask & -(value >> 15);
      result &= ~mask;
    }
    return result;
  }

  /// Round half-precision number to nearest integer value.
  /// @tparam R rounding mode to use, `std::round_indeterminate` for fastest
  /// rounding
  /// @param value binary representation of half-precision value
  /// @return half-precision bits for nearest integral value
  template <std::float_round_style R>
  std::uint16_t round_half(std::uint16_t value)
  {
    return round_half_impl<R, half_config::round_ties_to_even>(value);
  }

  /// Round half-precision number to nearest integer value using
  /// round-to-nearest-away-from-zero.
  /// @param value binary representation of half-precision value
  /// @return half-precision bits for nearest integral value
  inline std::uint16_t round_half_up(std::uint16_t value)
  {
    return round_half_impl<std::round_to_nearest, 0>(value);
  }
  /// @}

  struct functions;
  template <typename>
  struct unary_specialized;
  template <typename, typename>
  struct binary_specialized;
  template <typename, typename, std::float_round_style>
  struct half_caster;
}

/// Half-precision floating point type.
/// This class implements an IEEE-conformant half-precision floating point type
/// with the usual arithmetic operators and conversions. It is implicitly
/// convertible to single-precision floating point, which makes artihmetic
/// expressions and functions with mixed-type operands to be of the most precise
/// operand type. Additionally all arithmetic operations (and many mathematical
/// functions) are carried out in single-precision internally. All conversions
/// from single- to half-precision are done using the library's default rounding
/// mode, but temporary results inside chained arithmetic expressions are kept
/// in single-precision as long as possible (while of course still maintaining a
/// strong half-precision type).
///
/// According to the C++98/03 definition, the half type is not a POD type. But
/// according to C++11's less strict and extended definitions it is both a
/// standard layout type and a trivially copyable type (even if not a POD type),
/// which means it can be standard-conformantly copied using raw binary copies.
/// But in this context some more words about the actual size of the type.
/// Although the half is representing an IEEE 16-bit type, it does not
/// neccessarily have to be of exactly 16-bits size. But on any reasonable
/// implementation the actual binary representation of this type will most
/// probably not ivolve any additional "magic" or padding beyond the simple
/// binary representation of the underlying 16-bit IEEE number, even if not
/// strictly guaranteed by the standard. But even then it only has an actual
/// size of 16 bits if your C++ implementation supports an unsigned integer type
/// of exactly 16 bits width. But this should be the case on nearly any
/// reasonable platform.
///
/// So if your C++ implementation is not totally exotic or imposes special
/// alignment requirements, it is a reasonable assumption that the data of a
/// half is just comprised of the 2 bytes of the underlying IEEE representation.
class half
{
public:
  /// Default constructor.
  /// This initializes the half to 0. Although this does not match the builtin
  /// types' default-initialization semantics and may be less efficient than no
  /// initialization, it is needed to provide proper value-initialization
  /// semantics.
  half() noexcept = default;

  /// Copy constructor.
  /// @tparam T type of concrete half expression
  /// @param rhs half expression to copy from
  half(detail::expr rhs)
  : _data(detail::float2half<half_config::round_style>(static_cast<float>(rhs)))
  {
  }

  /// Conversion constructor.
  /// @param rhs float to convert
  half(float rhs) : _data(detail::float2half<half_config::round_style>(rhs))
  {
  }

  /// Conversion constructor.
  /// @param rhs double to convert
  half(double rhs) : _data(detail::float2half<half_config::round_style>(rhs))
  {
  }

  /// Conversion constructor.
  /// @param rhs int to convert
  template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
  half(T rhs) : _data(detail::int2half<half_config::round_style>(rhs))
  {
  }

  half(const half&) = default;
  half(half&&) = default;
  ~half() = default;

  /// Conversion to single-precision.
  /// @return single precision value representing expression value
  operator float() const
  {
    return detail::half2float<float>(_data);
  }

  half& operator=(const half&) = default;
  half& operator=(half&&) = default;

  /// Assignment operator.
  /// @param rhs single-precision value to copy from
  /// @return reference to this half
  half& operator=(float rhs)
  {
    _data = detail::float2half<half_config::round_style>(rhs);
    return *this;
  }

  /// Assignment operator.
  /// @param rhs double-precision value to copy from
  /// @return reference to this half
  half& operator=(double rhs)
  {
    _data = detail::float2half<half_config::round_style>(rhs);
    return *this;
  }

  /// Assignment operator.
  /// @param rhs int value to copy from
  /// @return reference to this half
  template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
  half& operator=(T rhs)
  {
    _data = detail::int2half<half_config::round_style>(rhs);
    return *this;
  }

  /// Assignment operator.
  /// @tparam T type of concrete half expression
  /// @param rhs half expression to copy from
  /// @return reference to this half
  half& operator=(detail::expr rhs)
  {
    return *this = static_cast<float>(rhs);
  }

  /// Arithmetic assignment.
  /// @tparam T type of concrete half expression
  /// @param rhs half expression to add
  /// @return reference to this half
  template <typename T>
  typename detail::enable<half&, T>::type operator+=(T rhs)
  {
    return *this += static_cast<float>(rhs);
  }

  /// Arithmetic assignment.
  /// @tparam T type of concrete half expression
  /// @param rhs half expression to subtract
  /// @return reference to this half
  template <typename T>
  typename detail::enable<half&, T>::type operator-=(T rhs)
  {
    return *this -= static_cast<float>(rhs);
  }

  /// Arithmetic assignment.
  /// @tparam T type of concrete half expression
  /// @param rhs half expression to multiply with
  /// @return reference to this half
  template <typename T>
  typename detail::enable<half&, T>::type operator*=(T rhs)
  {
    return *this *= static_cast<float>(rhs);
  }

  /// Arithmetic assignment.
  /// @tparam T type of concrete half expression
  /// @param rhs half expression to divide by
  /// @return reference to this half
  template <typename T>
  typename detail::enable<half&, T>::type operator/=(T rhs)
  {
    return *this /= static_cast<float>(rhs);
  }

  /// Arithmetic assignment.
  /// @param rhs single-precision value to add
  /// @return reference to this half
  half& operator+=(float rhs)
  {
    _data = detail::float2half<half_config::round_style>(
      detail::half2float<float>(_data) + rhs);
    return *this;
  }

  /// Arithmetic assignment.
  /// @param rhs single-precision value to subtract
  /// @return reference to this half
  half& operator-=(float rhs)
  {
    _data = detail::float2half<half_config::round_style>(
      detail::half2float<float>(_data) - rhs);
    return *this;
  }

  /// Arithmetic assignment.
  /// @param rhs single-precision value to multiply with
  /// @return reference to this half
  half& operator*=(float rhs)
  {
    _data = detail::float2half<half_config::round_style>(
      detail::half2float<float>(_data) * rhs);
    return *this;
  }

  /// Arithmetic assignment.
  /// @param rhs single-precision value to divide by
  /// @return reference to this half
  half& operator/=(float rhs)
  {
    _data = detail::float2half<half_config::round_style>(
      detail::half2float<float>(_data) / rhs);
    return *this;
  }

  /// Prefix increment.
  /// @return incremented half value
  half& operator++()
  {
    return *this += 1.0f;
  }

  /// Prefix decrement.
  /// @return decremented half value
  half& operator--()
  {
    return *this -= 1.0f;
  }

  /// Postfix increment.
  /// @return non-incremented half value
  half operator++(int)
  {
    half out(*this);
    ++*this;
    return out;
  }

  /// Postfix decrement.
  /// @return non-decremented half value
  half operator--(int)
  {
    half out(*this);
    --*this;
    return out;
  }

private:
  friend struct detail::functions;
  friend struct detail::unary_specialized<half>;
  friend struct detail::binary_specialized<half, half>;
  template <typename, typename, std::float_round_style>
  friend struct detail::half_caster;
  friend class std::numeric_limits<half>;
  friend struct std::hash<half>;
  friend half literals::operator""_h(long double);

  /// Constructor.
  /// @param bits binary representation to set half to
  constexpr half(detail::binary_t, std::uint16_t bits) noexcept : _data(bits)
  {
  }

  /// Internal binary representation
  std::uint16_t _data;
};

namespace literals
{
  /// Half literal.
  /// While this returns an actual half-precision value, half literals can
  /// unfortunately not be constant expressions due to rather involved
  /// conversions.
  /// @param value
  ///   The literal value.
  /// @return
  ///   A half of given value (if representable).
  inline half operator""_h(long double value)
  {
    return half(detail::binary, detail::float2half<half_config::round_style>(
                                  static_cast<float>(value)));
  }
}

namespace detail
{
  /// Wrapper implementing unspecialized half-precision functions.
  struct functions
  {
    /// Addition implementation.
    /// @param x first operand
    /// @param y second operand
    /// @return Half-precision sum stored in single-precision
    static expr plus(float x, float y)
    {
      return expr(x + y);
    }

    /// Subtraction implementation.
    /// @param x first operand
    /// @param y second operand
    /// @return Half-precision difference stored in single-precision
    static expr minus(float x, float y)
    {
      return expr(x - y);
    }

    /// Multiplication implementation.
    /// @param x first operand
    /// @param y second operand
    /// @return Half-precision product stored in single-precision
    static expr multiplies(float x, float y)
    {
      return expr(x * y);
    }

    /// Division implementation.
    /// @param x first operand
    /// @param y second operand
    /// @return Half-precision quotient stored in single-precision
    static expr divides(float x, float y)
    {
      return expr(x / y);
    }

    /// Output implementation.
    /// @param out stream to write to
    /// @param arg value to write
    /// @return reference to stream
    template <typename charT, typename traits>
    static std::basic_ostream<charT, traits>& write(
      std::basic_ostream<charT, traits>& out, float arg)
    {
      return out << arg;
    }

    /// Input implementation.
    /// @param in stream to read from
    /// @param arg half to read into
    /// @return reference to stream
    template <typename charT, typename traits>
    static std::basic_istream<charT, traits>& read(
      std::basic_istream<charT, traits>& in, half& arg)
    {
      float f;
      if (in >> f)
        arg = f;
      return in;
    }

    /// Modulo implementation.
    /// @param x first operand
    /// @param y second operand
    /// @return Half-precision division remainder stored in single-precision
    static expr fmod(float x, float y)
    {
      return expr(std::fmod(x, y));
    }

    /// Remainder implementation.
    /// @param x first operand
    /// @param y second operand
    /// @return Half-precision division remainder stored in single-precision
    static expr remainder(float x, float y)
    {
      return expr(std::remainder(x, y));
    }

    /// Remainder implementation.
    /// @param x first operand
    /// @param y second operand
    /// @param quo address to store quotient bits at
    /// @return Half-precision division remainder stored in single-precision
    static expr remquo(float x, float y, int* quo)
    {
      return expr(std::remquo(x, y, quo));
    }

    /// Positive difference implementation.
    /// @param x first operand
    /// @param y second operand
    /// @return Positive difference stored in single-precision
    static expr fdim(float x, float y)
    {
      return expr(std::fdim(x, y));
    }

    /// Fused multiply-add implementation.
    /// @param x first operand
    /// @param y second operand
    /// @param z third operand
    /// @return \a x * \a y + \a z stored in single-precision
    static expr fma(float x, float y, float z)
    {
      return expr(std::fma(x, y, z));
    }

    /// Get NaN.
    /// @return Half-precision quiet NaN
    static half nanh()
    {
      return half(binary, 0x7FFF);
    }

    /// Exponential implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr exp(float arg)
    {
      return expr(std::exp(arg));
    }

    /// Exponential implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr expm1(float arg)
    {
      return expr(std::expm1(arg));
    }

    /// Binary exponential implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr exp2(float arg)
    {
      return expr(std::exp2(arg));
    }

    /// Logarithm implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr log(float arg)
    {
      return expr(std::log(arg));
    }

    /// Common logarithm implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr log10(float arg)
    {
      return expr(std::log10(arg));
    }

    /// Logarithm implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr log1p(float arg)
    {
      return expr(std::log1p(arg));
    }

    /// Binary logarithm implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr log2(float arg)
    {
      return expr(std::log2(arg));
    }

    /// Square root implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr sqrt(float arg)
    {
      return expr(std::sqrt(arg));
    }

    /// Cubic root implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr cbrt(float arg)
    {
      return expr(std::cbrt(arg));
    }

    /// Hypotenuse implementation.
    /// @param x first argument
    /// @param y second argument
    /// @return function value stored in single-preicision
    static expr hypot(float x, float y)
    {
      return expr(std::hypot(x, y));
    }

    /// Power implementation.
    /// @param base value to exponentiate
    /// @param exp power to expontiate to
    /// @return function value stored in single-preicision
    static expr pow(float base, float exp)
    {
      return expr(std::pow(base, exp));
    }

    /// Sine implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr sin(float arg)
    {
      return expr(std::sin(arg));
    }

    /// Cosine implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr cos(float arg)
    {
      return expr(std::cos(arg));
    }

    /// Tan implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr tan(float arg)
    {
      return expr(std::tan(arg));
    }

    /// Arc sine implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr asin(float arg)
    {
      return expr(std::asin(arg));
    }

    /// Arc cosine implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr acos(float arg)
    {
      return expr(std::acos(arg));
    }

    /// Arc tangent implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr atan(float arg)
    {
      return expr(std::atan(arg));
    }

    /// Arc tangent implementation.
    /// @param x first argument
    /// @param y second argument
    /// @return function value stored in single-preicision
    static expr atan2(float x, float y)
    {
      return expr(std::atan2(x, y));
    }

    /// Hyperbolic sine implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr sinh(float arg)
    {
      return expr(std::sinh(arg));
    }

    /// Hyperbolic cosine implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr cosh(float arg)
    {
      return expr(std::cosh(arg));
    }

    /// Hyperbolic tangent implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr tanh(float arg)
    {
      return expr(std::tanh(arg));
    }

    /// Hyperbolic area sine implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr asinh(float arg)
    {
      return expr(std::asinh(arg));
    }

    /// Hyperbolic area cosine implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr acosh(float arg)
    {
      return expr(std::acosh(arg));
    }

    /// Hyperbolic area tangent implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr atanh(float arg)
    {
      return expr(std::atanh(arg));
    }

    /// Error function implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr erf(float arg)
    {
      return expr(std::erf(arg));
    }

    /// Complementary implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr erfc(float arg)
    {
      return expr(std::erfc(arg));
    }

    /// Gamma logarithm implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr lgamma(float arg)
    {
      return expr(std::lgamma(arg));
    }

    /// Gamma implementation.
    /// @param arg function argument
    /// @return function value stored in single-preicision
    static expr tgamma(float arg)
    {
      return expr(std::tgamma(arg));
    }

    /// Floor implementation.
    /// @param arg value to round
    /// @return rounded value
    static half floor(half arg)
    {
      return half(binary,
                  round_half<std::round_toward_neg_infinity>(arg._data));
    }

    /// Ceiling implementation.
    /// @param arg value to round
    /// @return rounded value
    static half ceil(half arg)
    {
      return half(binary, round_half<std::round_toward_infinity>(arg._data));
    }

    /// Truncation implementation.
    /// @param arg value to round
    /// @return rounded value
    static half trunc(half arg)
    {
      return half(binary, round_half<std::round_toward_zero>(arg._data));
    }

    /// Nearest integer implementation.
    /// @param arg value to round
    /// @return rounded value
    static half round(half arg)
    {
      return half(binary, round_half_up(arg._data));
    }

    /// Nearest integer implementation.
    /// @param arg value to round
    /// @return rounded value
    static long lround(half arg)
    {
      return detail::half2int_up<long>(arg._data);
    }

    /// Nearest integer implementation.
    /// @param arg value to round
    /// @return rounded value
    static half rint(half arg)
    {
      return half(binary, round_half<half_config::round_style>(arg._data));
    }

    /// Nearest integer implementation.
    /// @param arg value to round
    /// @return rounded value
    static long lrint(half arg)
    {
      return detail::half2int<half_config::round_style, long>(arg._data);
    }

    /// Nearest integer implementation.
    /// @param arg value to round
    /// @return rounded value
    static long long llround(half arg)
    {
      return detail::half2int_up<long long>(arg._data);
    }

    /// Nearest integer implementation.
    /// @param arg value to round
    /// @return rounded value
    static long long llrint(half arg)
    {
      return detail::half2int<half_config::round_style, long long>(arg._data);
    }

    /// Decompression implementation.
    /// @param arg number to decompress
    /// @param exp address to store exponent at
    /// @return normalized significant
    static half frexp(half arg, int* exp)
    {
      int m = arg._data & 0x7FFF;
      int e = -14;
      if (m >= 0x7C00 || !m)
      {
        *exp = 0;
        return arg;
      }
      for (; m < 0x400; m <<= 1, --e)
        ;
      *exp = e + (m >> 10);
      return half(binary, (arg._data & 0x8000) | 0x3800 | (m & 0x3FF));
    }

    /// Decompression implementation.
    /// @param arg number to decompress
    /// @param iptr address to store integer part at
    /// @return fractional part
    static half modf(half arg, half* iptr)
    {
      unsigned int e = arg._data & 0x7FFF;
      if (e >= 0x6400)
      {
        *iptr = arg;
        return half(binary, arg._data & (0x8000U | -(e > 0x7C00)));
      }
      if (e < 0x3C00)
      {
        iptr->_data = arg._data & 0x8000;
        return arg;
      }
      e >>= 10;
      unsigned int mask = (1 << (25 - e)) - 1, m = arg._data & mask;
      iptr->_data = arg._data & ~mask;
      if (!m)
        return half(binary, arg._data & 0x8000);
      for (; m < 0x400; m <<= 1)
        --e;
      return half(binary, static_cast<std::uint16_t>((arg._data & 0x8000) |
                                                     (e << 10) | (m & 0x3FF)));
    }

    /// Scaling implementation.
    /// @param arg number to scale
    /// @param exp power of two to scale by
    /// @return scaled number
    static half scalbln(half arg, long exp)
    {
      unsigned int m = arg._data & 0x7FFF;
      if (m >= 0x7C00 || !m)
        return arg;
      for (; m < 0x400; m <<= 1, --exp)
        ;
      exp += m >> 10;
      std::uint16_t value = arg._data & 0x8000;
      if (exp > 30)
      {
        if (half_config::round_style == std::round_toward_zero)
          value |= 0x7BFF;
        else if (half_config::round_style == std::round_toward_infinity)
          value |= 0x7C00 - (value >> 15);
        else if (half_config::round_style == std::round_toward_neg_infinity)
          value |= 0x7BFF + (value >> 15);
        else
          value |= 0x7C00;
      }
      else if (exp > 0)
        value |= (exp << 10) | (m & 0x3FF);
      else if (exp > -11)
      {
        m = (m & 0x3FF) | 0x400;
        if (half_config::round_style == std::round_to_nearest)
        {
          m += 1 << -exp;
          if constexpr (half_config::round_ties_to_even)
            m -= (m >> (1 - exp)) & 1;
        }
        else if (half_config::round_style == std::round_toward_infinity)
          m += ((value >> 15) - 1) & ((1 << (1 - exp)) - 1U);
        else if (half_config::round_style == std::round_toward_neg_infinity)
          m += -(value >> 15) & ((1 << (1 - exp)) - 1U);
        value |= m >> (1 - exp);
      }
      else if (half_config::round_style == std::round_toward_infinity)
        value -= (value >> 15) - 1;
      else if (half_config::round_style == std::round_toward_neg_infinity)
        value += value >> 15;
      return half(binary, value);
    }

    /// Exponent implementation.
    /// @param arg number to query
    /// @return floating point exponent
    static int ilogb(half arg)
    {
      int abs = arg._data & 0x7FFF;
      if (!abs)
        return FP_ILOGB0;
      if (abs < 0x7C00)
      {
        int exp = (abs >> 10) - 15;
        if (abs < 0x400)
          for (; abs < 0x200; abs <<= 1, --exp)
            ;
        return exp;
      }
      if (abs > 0x7C00)
        return FP_ILOGBNAN;
      return INT_MAX;
    }

    /// Exponent implementation.
    /// @param arg number to query
    /// @return floating point exponent
    static half logb(half arg)
    {
      int abs = arg._data & 0x7FFF;
      if (!abs)
        return half(binary, 0xFC00);
      if (abs < 0x7C00)
      {
        int exp = (abs >> 10) - 15;
        if (abs < 0x400)
        {
          for (; abs < 0x200; abs <<= 1)
            --exp;
        }
        auto bits = static_cast<std::uint16_t>(((exp < 0) ? 1 : 0) << 15);
        if (exp)
        {
          unsigned int m = static_cast<unsigned int>(std::abs(exp) << 6);
          unsigned int e = 18;
          for (; m < 0x400; m <<= 1, --e)
            ;
          bits |= (e << 10) + m;
        }
        return half(binary, bits);
      }
      if (abs > 0x7C00)
        return arg;
      return half(binary, 0x7C00);
    }

    /// Enumeration implementation.
    /// @param from number to increase/decrease
    /// @param to direction to enumerate into
    /// @return next representable number
    static half nextafter(half from, half to)
    {
      std::uint16_t fabs = from._data & 0x7FFF, tabs = to._data & 0x7FFF;
      if (fabs > 0x7C00)
        return from;
      if (tabs > 0x7C00 || from._data == to._data || !(fabs | tabs))
        return to;
      if (!fabs)
        return half(binary, (to._data & 0x8000) + 1);
      bool lt =
        ((fabs == from._data) ? static_cast<int>(fabs)
                              : -static_cast<int>(fabs)) <
        ((tabs == to._data) ? static_cast<int>(tabs) : -static_cast<int>(tabs));
      return half(
        binary, static_cast<std::uint16_t>(
                  from._data +
                  (((from._data >> 15) ^ static_cast<unsigned>(lt)) << 1) - 1));
    }

    /// Enumeration implementation.
    /// @param from number to increase/decrease
    /// @param to direction to enumerate into
    /// @return next representable number
    static half nexttoward(half from, long double to)
    {
      if (isnan(from))
        return from;
      auto lfrom = static_cast<long double>(from);
      if (std::isnan(to) || lfrom == to)
        return half(static_cast<float>(to));
      if (!(from._data & 0x7FFF))
        return half(binary,
                    static_cast<std::uint16_t>((std::signbit(to) << 15) + 1));
      return half(
        binary,
        static_cast<std::uint16_t>(
          from._data +
          (((from._data >> 15) ^ static_cast<unsigned>(lfrom < to)) << 1) - 1));
    }

    /// Sign implementation
    /// @param x first operand
    /// @param y second operand
    /// @return composed value
    static half copysign(half x, half y)
    {
      return half(binary, x._data ^ ((x._data ^ y._data) & 0x8000));
    }

    /// Classification implementation.
    /// @param arg value to classify
    /// @retval true if infinite number
    /// @retval false else
    static int fpclassify(half arg)
    {
      unsigned int abs = arg._data & 0x7FFF;
      return abs ? ((abs > 0x3FF) ? ((abs >= 0x7C00)
                                       ? ((abs > 0x7C00) ? FP_NAN : FP_INFINITE)
                                       : FP_NORMAL)
                                  : FP_SUBNORMAL)
                 : FP_ZERO;
    }

    /// Classification implementation.
    /// @param arg value to classify
    /// @retval true if finite number
    /// @retval false else
    static bool isfinite(half arg)
    {
      return (arg._data & 0x7C00) != 0x7C00;
    }

    /// Classification implementation.
    /// @param arg value to classify
    /// @retval true if infinite number
    /// @retval false else
    static bool isinf(half arg)
    {
      return (arg._data & 0x7FFF) == 0x7C00;
    }

    /// Classification implementation.
    /// @param arg value to classify
    /// @retval true if not a number
    /// @retval false else
    static bool isnan(half arg)
    {
      return (arg._data & 0x7FFF) > 0x7C00;
    }

    /// Classification implementation.
    /// @param arg value to classify
    /// @retval true if normal number
    /// @retval false else
    static bool isnormal(half arg)
    {
      return ((arg._data & 0x7C00) != 0) & ((arg._data & 0x7C00) != 0x7C00);
    }

    /// Sign bit implementation.
    /// @param arg value to check
    /// @retval true if signed
    /// @retval false if unsigned
    static bool signbit(half arg)
    {
      return (arg._data & 0x8000) != 0;
    }

    /// Comparison implementation.
    /// @param x first operand
    /// @param y second operand
    /// @retval true if operands equal
    /// @retval false else
    static bool isequal(half x, half y)
    {
      return (x._data == y._data || !((x._data | y._data) & 0x7FFF)) &&
             !isnan(x);
    }

    /// Comparison implementation.
    /// @param x first operand
    /// @param y second operand
    /// @retval true if operands not equal
    /// @retval false else
    static bool isnotequal(half x, half y)
    {
      return (x._data != y._data && ((x._data | y._data) & 0x7FFF)) || isnan(x);
    }

    /// Comparison implementation.
    /// @param x first operand
    /// @param y second operand
    /// @retval true if \a x > \a y
    /// @retval false else
    static bool isgreater(half x, half y)
    {
      int xabs = x._data & 0x7FFF, yabs = y._data & 0x7FFF;
      return xabs <= 0x7C00 && yabs <= 0x7C00 &&
             (((xabs == x._data) ? xabs : -xabs) >
              ((yabs == y._data) ? yabs : -yabs));
    }

    /// Comparison implementation.
    /// @param x first operand
    /// @param y second operand
    /// @retval true if \a x >= \a y
    /// @retval false else
    static bool isgreaterequal(half x, half y)
    {
      int xabs = x._data & 0x7FFF, yabs = y._data & 0x7FFF;
      return xabs <= 0x7C00 && yabs <= 0x7C00 &&
             (((xabs == x._data) ? xabs : -xabs) >=
              ((yabs == y._data) ? yabs : -yabs));
    }

    /// Comparison implementation.
    /// @param x first operand
    /// @param y second operand
    /// @retval true if \a x < \a y
    /// @retval false else
    static bool isless(half x, half y)
    {
      int xabs = x._data & 0x7FFF, yabs = y._data & 0x7FFF;
      return xabs <= 0x7C00 && yabs <= 0x7C00 &&
             (((xabs == x._data) ? xabs : -xabs) <
              ((yabs == y._data) ? yabs : -yabs));
    }

    /// Comparison implementation.
    /// @param x first operand
    /// @param y second operand
    /// @retval true if \a x <= \a y
    /// @retval false else
    static bool islessequal(half x, half y)
    {
      int xabs = x._data & 0x7FFF, yabs = y._data & 0x7FFF;
      return xabs <= 0x7C00 && yabs <= 0x7C00 &&
             (((xabs == x._data) ? xabs : -xabs) <=
              ((yabs == y._data) ? yabs : -yabs));
    }

    /// Comparison implementation.
    /// @param x first operand
    /// @param y second operand
    /// @retval true if either \a x > \a y nor \a x < \a y
    /// @retval false else
    static bool islessgreater(half x, half y)
    {
      int xabs = x._data & 0x7FFF, yabs = y._data & 0x7FFF;
      if (xabs > 0x7C00 || yabs > 0x7C00)
        return false;
      int a = (xabs == x._data) ? xabs : -xabs,
          b = (yabs == y._data) ? yabs : -yabs;
      return a < b || a > b;
    }

    /// Comparison implementation.
    /// @param x first operand
    /// @param y second operand
    /// @retval true if operand unordered
    /// @retval false else
    static bool isunordered(half x, half y)
    {
      return isnan(x) || isnan(y);
    }

  private:
    static double erf(double arg)
    {
      if (std::isinf(arg))
        return (arg < 0.0) ? -1.0 : 1.0;
      double x2 = arg * arg, ax2 = 0.147 * x2,
             value = std::sqrt(
               1.0 - std::exp(-x2 * (1.2732395447351626861510701069801 + ax2) /
                              (1.0 + ax2)));
      return std::signbit(arg) ? -value : value;
    }

    static double lgamma(double arg)
    {
      double v = 1.0;
      for (; arg < 8.0; ++arg)
        v *= arg;
      double w = 1.0 / (arg * arg);
      return (((((((-0.02955065359477124183006535947712 * w +
                    0.00641025641025641025641025641026) *
                     w +
                   -0.00191752691752691752691752691753) *
                    w +
                  8.4175084175084175084175084175084e-4) *
                   w +
                 -5.952380952380952380952380952381e-4) *
                  w +
                7.9365079365079365079365079365079e-4) *
                 w +
               -0.00277777777777777777777777777778) *
                w +
              0.08333333333333333333333333333333) /
               arg +
             0.91893853320467274178032973640562 - std::log(v) - arg +
             (arg - 0.5) * std::log(arg);
    }
  };

  /// Wrapper for unary half-precision functions needing specialization for
  /// individual argument types.
  /// @tparam T argument type
  template <typename T>
  struct unary_specialized
  {
    /// Negation implementation.
    /// @param arg value to negate
    /// @return negated value
    static constexpr half negate(half arg)
    {
      return half(binary, arg._data ^ 0x8000);
    }

    /// Absolute value implementation.
    /// @param arg function argument
    /// @return absolute value
    static half fabs(half arg)
    {
      return half(binary, arg._data & 0x7FFF);
    }
  };
  template <>
  struct unary_specialized<expr>
  {
    static constexpr expr negate(float arg)
    {
      return expr(-arg);
    }
    static expr fabs(float arg)
    {
      return expr(std::fabs(arg));
    }
  };

  /// Wrapper for binary half-precision functions needing specialization for
  /// individual argument types.
  /// @tparam T first argument type
  /// @tparam U first argument type
  template <typename T, typename U>
  struct binary_specialized
  {
    /// Minimum implementation.
    /// @param x first operand
    /// @param y second operand
    /// @return minimum value
    static expr fmin(float x, float y)
    {
      return expr(std::fmin(x, y));
    }

    /// Maximum implementation.
    /// @param x first operand
    /// @param y second operand
    /// @return maximum value
    static expr fmax(float x, float y)
    {
      return expr(std::fmax(x, y));
    }
  };
  template <>
  struct binary_specialized<half, half>
  {
    static half fmin(half x, half y)
    {
      int xabs = x._data & 0x7FFF, yabs = y._data & 0x7FFF;
      if (xabs > 0x7C00)
        return y;
      if (yabs > 0x7C00)
        return x;
      return (((xabs == x._data) ? xabs : -xabs) >
              ((yabs == y._data) ? yabs : -yabs))
               ? y
               : x;
    }
    static half fmax(half x, half y)
    {
      int xabs = x._data & 0x7FFF, yabs = y._data & 0x7FFF;
      if (xabs > 0x7C00)
        return y;
      if (yabs > 0x7C00)
        return x;
      return (((xabs == x._data) ? xabs : -xabs) <
              ((yabs == y._data) ? yabs : -yabs))
               ? y
               : x;
    }
  };

  /// Helper class for half casts.
  /// This class template has to be specialized for all valid cast argument to
  /// define an appropriate static `cast` member function and a corresponding
  /// `type` member denoting its return type.
  /// @tparam T destination type
  /// @tparam U source type
  /// @tparam R rounding mode to use
  template <typename T, typename U,
            std::float_round_style R = std::round_indeterminate>
  struct half_caster
  {
  };
  template <typename U, std::float_round_style R>
  struct half_caster<half, U, R>
  {
    static_assert(std::is_arithmetic_v<U>,
                  "half_cast from non-arithmetic type unsupported");

    static half cast(U arg)
    {
      return cast_impl(arg, std::is_floating_point<U>());
    }

  private:
    static half cast_impl(U arg, true_type)
    {
      return half(binary, float2half<R>(arg));
    }

    static half cast_impl(U arg, false_type)
    {
      return half(binary, int2half<R>(arg));
    }
  };

  template <typename T, std::float_round_style R>
  struct half_caster<T, half, R>
  {
    static_assert(std::is_arithmetic_v<T>,
                  "half_cast to non-arithmetic type unsupported");

    static T cast(half arg)
    {
      return cast_impl(arg, std::is_floating_point<T>());
    }

  private:
    static T cast_impl(half arg, true_type)
    {
      return half2float<T>(arg._data);
    }

    static T cast_impl(half arg, false_type)
    {
      return half2int<R, T>(arg._data);
    }
  };

  template <typename T, std::float_round_style R>
  struct half_caster<T, expr, R>
  {
    static_assert(std::is_arithmetic_v<T>,
                  "half_cast to non-arithmetic type unsupported");

    static T cast(expr arg)
    {
      return cast_impl(arg, std::is_floating_point<T>());
    }

  private:
    static T cast_impl(float arg, true_type)
    {
      return static_cast<T>(arg);
    }

    static T cast_impl(half arg, false_type)
    {
      return half2int<R, T>(arg._data);
    }
  };

  template <std::float_round_style R>
  struct half_caster<half, half, R>
  {
    static half cast(half arg)
    {
      return arg;
    }
  };

  template <std::float_round_style R>
  struct half_caster<half, expr, R> : half_caster<half, half, R>
  {
  };

  /// @name Comparison operators
  /// @{

  /// Comparison for equality.
  /// @param x first operand
  /// @param y second operand
  /// @retval true if operands equal
  /// @retval false else
  template <typename T, typename U>
  enable_t<bool, T, U> operator==(T x, U y)
  {
    return functions::isequal(x, y);
  }

  /// Comparison for inequality.
  /// @param x first operand
  /// @param y second operand
  /// @retval true if operands not equal
  /// @retval false else
  template <typename T, typename U>
  enable_t<bool, T, U> operator!=(T x, U y)
  {
    return functions::isnotequal(x, y);
  }

  /// Comparison for less than.
  /// @param x first operand
  /// @param y second operand
  /// @retval true if \a x less than \a y
  /// @retval false else
  template <typename T, typename U>
  enable_t<bool, T, U> operator<(T x, U y)
  {
    return functions::isless(x, y);
  }

  /// Comparison for greater than.
  /// @param x first operand
  /// @param y second operand
  /// @retval true if \a x greater than \a y
  /// @retval false else
  template <typename T, typename U>
  enable_t<bool, T, U> operator>(T x, U y)
  {
    return functions::isgreater(x, y);
  }

  /// Comparison for less equal.
  /// @param x first operand
  /// @param y second operand
  /// @retval true if \a x less equal \a y
  /// @retval false else
  template <typename T, typename U>
  enable_t<bool, T, U> operator<=(T x, U y)
  {
    return functions::islessequal(x, y);
  }

  /// Comparison for greater equal.
  /// @param x first operand
  /// @param y second operand
  /// @retval true if \a x greater equal \a y
  /// @retval false else
  template <typename T, typename U>
  enable_t<bool, T, U> operator>=(T x, U y)
  {
    return functions::isgreaterequal(x, y);
  }

  /// @}
  /// @name Arithmetic operators
  /// @{

  /// Add halfs.
  /// @param x left operand
  /// @param y right operand
  /// @return sum of half expressions
  template <typename T, typename U>
  enable_t<expr, T, U> operator+(T x, U y)
  {
    return functions::plus(x, y);
  }

  /// Subtract halfs.
  /// @param x left operand
  /// @param y right operand
  /// @return difference of half expressions
  template <typename T, typename U>
  enable_t<expr, T, U> operator-(T x, U y)
  {
    return functions::minus(x, y);
  }

  /// Multiply halfs.
  /// @param x left operand
  /// @param y right operand
  /// @return product of half expressions
  template <typename T, typename U>
  enable_t<expr, T, U> operator*(T x, U y)
  {
    return functions::multiplies(x, y);
  }

  /// Divide halfs.
  /// @param x left operand
  /// @param y right operand
  /// @return quotient of half expressions
  template <typename T, typename U>
  enable_t<expr, T, U> operator/(T x, U y)
  {
    return functions::divides(x, y);
  }

  /// Identity.
  /// @param arg operand
  /// @return uncahnged operand
  template <typename T>
  constexpr enable_t<T, T> operator+(T arg)
  {
    return arg;
  }

  /// Negation.
  /// @param arg operand
  /// @return negated operand
  template <typename T>
  constexpr enable_t<T, T> operator-(T arg)
  {
    return unary_specialized<T>::negate(arg);
  }

  /// @}
  /// @name Input and output
  /// @{

  /// Output operator.
  /// @param out output stream to write into
  /// @param arg half expression to write
  /// @return reference to output stream
  template <typename T, typename charT, typename traits>
  enable_t<std::basic_ostream<charT, traits>&, T> operator<<(
    std::basic_ostream<charT, traits>& out, T arg)
  {
    return functions::write(out, arg);
  }

  /// Input operator.
  /// @param in input stream to read from
  /// @param arg half to read into
  /// @return reference to input stream
  template <typename charT, typename traits>
  std::basic_istream<charT, traits>& operator>>(
    std::basic_istream<charT, traits>& in, half& arg)
  {
    return functions::read(in, arg);
  }

  /// @}
  /// @name Basic mathematical operations
  /// @{

  /// Absolute value.
  /// @param arg operand
  /// @return absolute value of \a arg
  //    template<typename T> enable_t<T, T> abs(T arg) { return
  //    unary_specialized<T>::fabs(arg); }
  inline half abs(half arg)
  {
    return unary_specialized<half>::fabs(arg);
  }
  inline expr abs(expr arg)
  {
    return unary_specialized<expr>::fabs(arg);
  }

  /// Absolute value.
  /// @param arg operand
  /// @return absolute value of \a arg
  //    template<typename T> enable_t<T, T> fabs(T arg) { return
  //    unary_specialized<T>::fabs(arg); }
  inline half fabs(half arg)
  {
    return unary_specialized<half>::fabs(arg);
  }
  inline expr fabs(expr arg)
  {
    return unary_specialized<expr>::fabs(arg);
  }

  /// Remainder of division.
  /// @param x first operand
  /// @param y second operand
  /// @return remainder of floating point division.
  //    template<typename T,typename U> enable_t<expr,T,U> fmod(T
  //    x, U y) { return functions::fmod(x, y); }
  inline expr fmod(half x, half y)
  {
    return functions::fmod(x, y);
  }
  inline expr fmod(half x, expr y)
  {
    return functions::fmod(x, y);
  }
  inline expr fmod(expr x, half y)
  {
    return functions::fmod(x, y);
  }
  inline expr fmod(expr x, expr y)
  {
    return functions::fmod(x, y);
  }

  /// Remainder of division.
  /// @param x first operand
  /// @param y second operand
  /// @return remainder of floating point division.
  //    template<typename T,typename U> enable_t<expr,T,U>
  //    remainder(T x, U y) { return functions::remainder(x, y); }
  inline expr remainder(half x, half y)
  {
    return functions::remainder(x, y);
  }
  inline expr remainder(half x, expr y)
  {
    return functions::remainder(x, y);
  }
  inline expr remainder(expr x, half y)
  {
    return functions::remainder(x, y);
  }
  inline expr remainder(expr x, expr y)
  {
    return functions::remainder(x, y);
  }

  /// Remainder of division.
  /// @param x first operand
  /// @param y second operand
  /// @param quo address to store some bits of quotient at
  /// @return remainder of floating point division.
  //    template<typename T,typename U> enable_t<expr,T,U> remquo(T
  //    x, U y, int *quo) { return functions::remquo(x, y, quo); }
  inline expr remquo(half x, half y, int* quo)
  {
    return functions::remquo(x, y, quo);
  }
  inline expr remquo(half x, expr y, int* quo)
  {
    return functions::remquo(x, y, quo);
  }
  inline expr remquo(expr x, half y, int* quo)
  {
    return functions::remquo(x, y, quo);
  }
  inline expr remquo(expr x, expr y, int* quo)
  {
    return functions::remquo(x, y, quo);
  }

  /// Fused multiply add.
  /// @param x first operand
  /// @param y second operand
  /// @param z third operand
  /// @return ( \a x * \a y ) + \a z rounded as one operation.
  //    template<typename T,typename U,typename V> typename
  //    enable<expr,T,U,V>::type fma(T x, U y, V z) { return functions::fma(x,
  //    y, z); }
  inline expr fma(half x, half y, half z)
  {
    return functions::fma(x, y, z);
  }
  inline expr fma(half x, half y, expr z)
  {
    return functions::fma(x, y, z);
  }
  inline expr fma(half x, expr y, half z)
  {
    return functions::fma(x, y, z);
  }
  inline expr fma(half x, expr y, expr z)
  {
    return functions::fma(x, y, z);
  }
  inline expr fma(expr x, half y, half z)
  {
    return functions::fma(x, y, z);
  }
  inline expr fma(expr x, half y, expr z)
  {
    return functions::fma(x, y, z);
  }
  inline expr fma(expr x, expr y, half z)
  {
    return functions::fma(x, y, z);
  }
  inline expr fma(expr x, expr y, expr z)
  {
    return functions::fma(x, y, z);
  }

  /// Maximum of half expressions.
  /// @param x first operand
  /// @param y second operand
  /// @return maximum of operands
  //    template<typename T,typename U> typename result<T,U>::type fmax(T x, U
  //    y) { return binary_specialized<T,U>::fmax(x, y); }
  inline half fmax(half x, half y)
  {
    return binary_specialized<half, half>::fmax(x, y);
  }
  inline expr fmax(half x, expr y)
  {
    return binary_specialized<half, expr>::fmax(x, y);
  }
  inline expr fmax(expr x, half y)
  {
    return binary_specialized<expr, half>::fmax(x, y);
  }
  inline expr fmax(expr x, expr y)
  {
    return binary_specialized<expr, expr>::fmax(x, y);
  }

  /// Minimum of half expressions.
  /// @param x first operand
  /// @param y second operand
  /// @return minimum of operands
  //    template<typename T,typename U> typename result<T,U>::type fmin(T x, U
  //    y) { return binary_specialized<T,U>::fmin(x, y); }
  inline half fmin(half x, half y)
  {
    return binary_specialized<half, half>::fmin(x, y);
  }
  inline expr fmin(half x, expr y)
  {
    return binary_specialized<half, expr>::fmin(x, y);
  }
  inline expr fmin(expr x, half y)
  {
    return binary_specialized<expr, half>::fmin(x, y);
  }
  inline expr fmin(expr x, expr y)
  {
    return binary_specialized<expr, expr>::fmin(x, y);
  }

  /// Positive difference.
  /// @param x first operand
  /// @param y second operand
  /// @return \a x - \a y or 0 if difference negative
  //    template<typename T,typename U> enable_t<expr,T,U> fdim(T
  //    x, U y) { return functions::fdim(x, y); }
  inline expr fdim(half x, half y)
  {
    return functions::fdim(x, y);
  }
  inline expr fdim(half x, expr y)
  {
    return functions::fdim(x, y);
  }
  inline expr fdim(expr x, half y)
  {
    return functions::fdim(x, y);
  }
  inline expr fdim(expr x, expr y)
  {
    return functions::fdim(x, y);
  }

  /// Get NaN value.
  /// @return quiet NaN
  inline half nanh(const char*)
  {
    return functions::nanh();
  }

  /// @}
  /// @name Exponential functions
  /// @{

  /// Exponential function.
  /// @param arg function argument
  /// @return e raised to \a arg
  //    template<typename T> enable_t<expr,T> exp(T arg) { return
  //    functions::exp(arg); }
  inline expr exp(half arg)
  {
    return functions::exp(arg);
  }
  inline expr exp(expr arg)
  {
    return functions::exp(arg);
  }

  /// Exponential minus one.
  /// @param arg function argument
  /// @return e raised to \a arg subtracted by 1
  //    template<typename T> enable_t<expr,T> expm1(T arg) { return
  //    functions::expm1(arg); }
  inline expr expm1(half arg)
  {
    return functions::expm1(arg);
  }
  inline expr expm1(expr arg)
  {
    return functions::expm1(arg);
  }

  /// Binary exponential.
  /// @param arg function argument
  /// @return 2 raised to \a arg
  //    template<typename T> enable_t<expr,T> exp2(T arg) { return
  //    functions::exp2(arg); }
  inline expr exp2(half arg)
  {
    return functions::exp2(arg);
  }
  inline expr exp2(expr arg)
  {
    return functions::exp2(arg);
  }

  /// Natural logorithm.
  /// @param arg function argument
  /// @return logarithm of \a arg to base e
  //    template<typename T> enable_t<expr,T> log(T arg) { return
  //    functions::log(arg); }
  inline expr log(half arg)
  {
    return functions::log(arg);
  }
  inline expr log(expr arg)
  {
    return functions::log(arg);
  }

  /// Common logorithm.
  /// @param arg function argument
  /// @return logarithm of \a arg to base 10
  //    template<typename T> enable_t<expr,T> log10(T arg) { return
  //    functions::log10(arg); }
  inline expr log10(half arg)
  {
    return functions::log10(arg);
  }
  inline expr log10(expr arg)
  {
    return functions::log10(arg);
  }

  /// Natural logorithm.
  /// @param arg function argument
  /// @return logarithm of \a arg plus 1 to base e
  //    template<typename T> enable_t<expr,T> log1p(T arg) { return
  //    functions::log1p(arg); }
  inline expr log1p(half arg)
  {
    return functions::log1p(arg);
  }
  inline expr log1p(expr arg)
  {
    return functions::log1p(arg);
  }

  /// Binary logorithm.
  /// @param arg function argument
  /// @return logarithm of \a arg to base 2
  //    template<typename T> enable_t<expr,T> log2(T arg) { return
  //    functions::log2(arg); }
  inline expr log2(half arg)
  {
    return functions::log2(arg);
  }
  inline expr log2(expr arg)
  {
    return functions::log2(arg);
  }

  /// @}
  /// @name Power functions
  /// @{

  /// Square root.
  /// @param arg function argument
  /// @return square root of \a arg
  //    template<typename T> enable_t<expr, T> sqrt(T arg) { return
  //    functions::sqrt(arg); }
  inline expr sqrt(half arg)
  {
    return functions::sqrt(arg);
  }
  inline expr sqrt(expr arg)
  {
    return functions::sqrt(arg);
  }

  /// Cubic root.
  /// @param arg function argument
  /// @return cubic root of \a arg
  //    template<typename T> enable_t<expr, T> cbrt(T arg) { return
  //    functions::cbrt(arg); }
  inline expr cbrt(half arg)
  {
    return functions::cbrt(arg);
  }
  inline expr cbrt(expr arg)
  {
    return functions::cbrt(arg);
  }

  /// Hypotenuse function.
  /// @param x first argument
  /// @param y second argument
  /// @return square root of sum of squares without internal over- or underflows
  //    template<typename T,typename U> enable_t<expr, T, U> hypot(T
  //    x, U y) { return functions::hypot(x, y); }
  inline expr hypot(half x, half y)
  {
    return functions::hypot(x, y);
  }
  inline expr hypot(half x, expr y)
  {
    return functions::hypot(x, y);
  }
  inline expr hypot(expr x, half y)
  {
    return functions::hypot(x, y);
  }
  inline expr hypot(expr x, expr y)
  {
    return functions::hypot(x, y);
  }

  /// Power function.
  /// @param base first argument
  /// @param exp second argument
  /// @return \a base raised to \a exp
  //    template<typename T,typename U> enable_t<expr, T, U> pow(T
  //    base, U exp) { return functions::pow(base, exp); }
  inline expr pow(half base, half exp)
  {
    return functions::pow(base, exp);
  }
  inline expr pow(half base, expr exp)
  {
    return functions::pow(base, exp);
  }
  inline expr pow(expr base, half exp)
  {
    return functions::pow(base, exp);
  }
  inline expr pow(expr base, expr exp)
  {
    return functions::pow(base, exp);
  }

  /// @}
  /// @name Trigonometric functions
  /// @{

  /// Sine function.
  /// @param arg function argument
  /// @return sine value of \a arg
  //    template<typename T> enable_t<expr, T> sin(T arg) { return
  //    functions::sin(arg); }
  inline expr sin(half arg)
  {
    return functions::sin(arg);
  }
  inline expr sin(expr arg)
  {
    return functions::sin(arg);
  }

  /// Cosine function.
  /// @param arg function argument
  /// @return cosine value of \a arg
  //    template<typename T> enable_t<expr, T> cos(T arg) { return
  //    functions::cos(arg); }
  inline expr cos(half arg)
  {
    return functions::cos(arg);
  }
  inline expr cos(expr arg)
  {
    return functions::cos(arg);
  }

  /// Tangent function.
  /// @param arg function argument
  /// @return tangent value of \a arg
  //    template<typename T> enable_t<expr, T> tan(T arg) { return
  //    functions::tan(arg); }
  inline expr tan(half arg)
  {
    return functions::tan(arg);
  }
  inline expr tan(expr arg)
  {
    return functions::tan(arg);
  }

  /// Arc sine.
  /// @param arg function argument
  /// @return arc sine value of \a arg
  //    template<typename T> enable_t<expr, T> asin(T arg) { return
  //    functions::asin(arg); }
  inline expr asin(half arg)
  {
    return functions::asin(arg);
  }
  inline expr asin(expr arg)
  {
    return functions::asin(arg);
  }

  /// Arc cosine function.
  /// @param arg function argument
  /// @return arc cosine value of \a arg
  //    template<typename T> enable_t<expr, T> acos(T arg) { return
  //    functions::acos(arg); }
  inline expr acos(half arg)
  {
    return functions::acos(arg);
  }
  inline expr acos(expr arg)
  {
    return functions::acos(arg);
  }

  /// Arc tangent function.
  /// @param arg function argument
  /// @return arc tangent value of \a arg
  //    template<typename T> enable_t<expr, T> atan(T arg) { return
  //    functions::atan(arg); }
  inline expr atan(half arg)
  {
    return functions::atan(arg);
  }
  inline expr atan(expr arg)
  {
    return functions::atan(arg);
  }

  /// Arc tangent function.
  /// @param x first argument
  /// @param y second argument
  /// @return arc tangent value
  //    template<typename T,typename U> enable_t<expr, T, U> atan2(T
  //    x, U y) { return functions::atan2(x, y); }
  inline expr atan2(half x, half y)
  {
    return functions::atan2(x, y);
  }
  inline expr atan2(half x, expr y)
  {
    return functions::atan2(x, y);
  }
  inline expr atan2(expr x, half y)
  {
    return functions::atan2(x, y);
  }
  inline expr atan2(expr x, expr y)
  {
    return functions::atan2(x, y);
  }

  /// @}
  /// @name Hyperbolic functions
  /// @{

  /// Hyperbolic sine.
  /// @param arg function argument
  /// @return hyperbolic sine value of \a arg
  //    template<typename T> enable_t<expr, T> sinh(T arg) { return
  //    functions::sinh(arg); }
  inline expr sinh(half arg)
  {
    return functions::sinh(arg);
  }
  inline expr sinh(expr arg)
  {
    return functions::sinh(arg);
  }

  /// Hyperbolic cosine.
  /// @param arg function argument
  /// @return hyperbolic cosine value of \a arg
  //    template<typename T> enable_t<expr, T> cosh(T arg) { return
  //    functions::cosh(arg); }
  inline expr cosh(half arg)
  {
    return functions::cosh(arg);
  }
  inline expr cosh(expr arg)
  {
    return functions::cosh(arg);
  }

  /// Hyperbolic tangent.
  /// @param arg function argument
  /// @return hyperbolic tangent value of \a arg
  //    template<typename T> enable_t<expr, T> tanh(T arg) { return
  //    functions::tanh(arg); }
  inline expr tanh(half arg)
  {
    return functions::tanh(arg);
  }
  inline expr tanh(expr arg)
  {
    return functions::tanh(arg);
  }

  /// Hyperbolic area sine.
  /// @param arg function argument
  /// @return area sine value of \a arg
  //    template<typename T> enable_t<expr, T> asinh(T arg) { return
  //    functions::asinh(arg); }
  inline expr asinh(half arg)
  {
    return functions::asinh(arg);
  }
  inline expr asinh(expr arg)
  {
    return functions::asinh(arg);
  }

  /// Hyperbolic area cosine.
  /// @param arg function argument
  /// @return area cosine value of \a arg
  //    template<typename T> enable_t<expr, T> acosh(T arg) { return
  //    functions::acosh(arg); }
  inline expr acosh(half arg)
  {
    return functions::acosh(arg);
  }
  inline expr acosh(expr arg)
  {
    return functions::acosh(arg);
  }

  /// Hyperbolic area tangent.
  /// @param arg function argument
  /// @return area tangent value of \a arg
  //    template<typename T> enable_t<expr, T> atanh(T arg) { return
  //    functions::atanh(arg); }
  inline expr atanh(half arg)
  {
    return functions::atanh(arg);
  }
  inline expr atanh(expr arg)
  {
    return functions::atanh(arg);
  }

  /// @}
  /// @name Error and gamma functions
  /// @{

  /// Error function.
  /// @param arg function argument
  /// @return error function value of \a arg
  //    template<typename T> enable_t<expr, T> erf(T arg) { return
  //    functions::erf(arg); }
  inline expr erf(half arg)
  {
    return functions::erf(arg);
  }
  inline expr erf(expr arg)
  {
    return functions::erf(arg);
  }

  /// Complementary error function.
  /// @param arg function argument
  /// @return 1 minus error function value of \a arg
  //    template<typename T> enable_t<expr, T> erfc(T arg) { return
  //    functions::erfc(arg); }
  inline expr erfc(half arg)
  {
    return functions::erfc(arg);
  }
  inline expr erfc(expr arg)
  {
    return functions::erfc(arg);
  }

  /// Natural logarithm of gamma function.
  /// @param arg function argument
  /// @return natural logarith of gamma function for \a arg
  //    template<typename T> enable_t<expr, T> lgamma(T arg) {
  //    return functions::lgamma(arg); }
  inline expr lgamma(half arg)
  {
    return functions::lgamma(arg);
  }
  inline expr lgamma(expr arg)
  {
    return functions::lgamma(arg);
  }

  /// Gamma function.
  /// @param arg function argument
  /// @return gamma function value of \a arg
  //    template<typename T> enable_t<expr, T> tgamma(T arg) {
  //    return functions::tgamma(arg); }
  inline expr tgamma(half arg)
  {
    return functions::tgamma(arg);
  }
  inline expr tgamma(expr arg)
  {
    return functions::tgamma(arg);
  }

  /// @}
  /// @name Rounding
  /// @{

  /// Nearest integer not less than half value.
  /// @param arg half to round
  /// @return nearest integer not less than \a arg
  //    template<typename T> enable_t<half, T> ceil(T arg) { return
  //    functions::ceil(arg); }
  inline half ceil(half arg)
  {
    return functions::ceil(arg);
  }
  inline half ceil(expr arg)
  {
    return functions::ceil(arg);
  }

  /// Nearest integer not greater than half value.
  /// @param arg half to round
  /// @return nearest integer not greater than \a arg
  //    template<typename T> enable_t<half, T> floor(T arg) { return
  //    functions::floor(arg); }
  inline half floor(half arg)
  {
    return functions::floor(arg);
  }
  inline half floor(expr arg)
  {
    return functions::floor(arg);
  }

  /// Nearest integer not greater in magnitude than half value.
  /// @param arg half to round
  /// @return nearest integer not greater in magnitude than \a arg
  //    template<typename T> enable_t<half, T> trunc(T arg) { return
  //    functions::trunc(arg); }
  inline half trunc(half arg)
  {
    return functions::trunc(arg);
  }
  inline half trunc(expr arg)
  {
    return functions::trunc(arg);
  }

  /// Nearest integer.
  /// @param arg half to round
  /// @return nearest integer, rounded away from zero in half-way cases
  //    template<typename T> enable_t<half, T> round(T arg) { return
  //    functions::round(arg); }
  inline half round(half arg)
  {
    return functions::round(arg);
  }
  inline half round(expr arg)
  {
    return functions::round(arg);
  }

  /// Nearest integer.
  /// @param arg half to round
  /// @return nearest integer, rounded away from zero in half-way cases
  //    template<typename T> enable_t<long,T> lround(T arg) {
  //    return functions::lround(arg); }
  inline long lround(half arg)
  {
    return functions::lround(arg);
  }
  inline long lround(expr arg)
  {
    return functions::lround(arg);
  }

  /// Nearest integer using half's internal rounding mode.
  /// @param arg half expression to round
  /// @return nearest integer using default rounding mode
  //    template<typename T> enable_t<half, T> nearbyint(T arg) {
  //    return functions::nearbyint(arg); }
  inline half nearbyint(half arg)
  {
    return functions::rint(arg);
  }
  inline half nearbyint(expr arg)
  {
    return functions::rint(arg);
  }

  /// Nearest integer using half's internal rounding mode.
  /// @param arg half expression to round
  /// @return nearest integer using default rounding mode
  //    template<typename T> enable_t<half, T> rint(T arg) { return
  //    functions::rint(arg); }
  inline half rint(half arg)
  {
    return functions::rint(arg);
  }
  inline half rint(expr arg)
  {
    return functions::rint(arg);
  }

  /// Nearest integer using half's internal rounding mode.
  /// @param arg half expression to round
  /// @return nearest integer using default rounding mode
  //    template<typename T> enable_t<long,T> lrint(T arg) { return
  //    functions::lrint(arg); }
  inline long lrint(half arg)
  {
    return functions::lrint(arg);
  }
  inline long lrint(expr arg)
  {
    return functions::lrint(arg);
  }
  /// Nearest integer.
  /// @param arg half to round
  /// @return nearest integer, rounded away from zero in half-way cases
  //    template<typename T> enable_t<long long,T> llround(T arg) {
  //    return functions::llround(arg); }
  inline long long llround(half arg)
  {
    return functions::llround(arg);
  }
  inline long long llround(expr arg)
  {
    return functions::llround(arg);
  }

  /// Nearest integer using half's internal rounding mode.
  /// @param arg half expression to round
  /// @return nearest integer using default rounding mode
  //    template<typename T> enable_t<long long,T> llrint(T arg) {
  //    return functions::llrint(arg); }
  inline long long llrint(half arg)
  {
    return functions::llrint(arg);
  }
  inline long long llrint(expr arg)
  {
    return functions::llrint(arg);
  }

  /// @}
  /// @name Floating point manipulation
  /// @{

  /// Decompress floating point number.
  /// @param arg number to decompress
  /// @param exp address to store exponent at
  /// @return significant in range [0.5, 1)
  //    template<typename T> enable_t<half, T> frexp(T arg, int
  //    *exp) { return functions::frexp(arg, exp); }
  inline half frexp(half arg, int* exp)
  {
    return functions::frexp(arg, exp);
  }
  inline half frexp(expr arg, int* exp)
  {
    return functions::frexp(arg, exp);
  }

  /// Multiply by power of two.
  /// @param arg number to modify
  /// @param exp power of two to multiply with
  /// @return \a arg multplied by 2 raised to \a exp
  //    template<typename T> enable_t<half, T> ldexp(T arg, int exp)
  //    { return functions::scalbln(arg, exp); }
  inline half ldexp(half arg, int exp)
  {
    return functions::scalbln(arg, exp);
  }
  inline half ldexp(expr arg, int exp)
  {
    return functions::scalbln(arg, exp);
  }

  /// Extract integer and fractional parts.
  /// @param arg number to decompress
  /// @param iptr address to store integer part at
  /// @return fractional part
  //    template<typename T> enable_t<half, T> modf(T arg, half
  //    *iptr) { return functions::modf(arg, iptr); }
  inline half modf(half arg, half* iptr)
  {
    return functions::modf(arg, iptr);
  }
  inline half modf(expr arg, half* iptr)
  {
    return functions::modf(arg, iptr);
  }

  /// Multiply by power of two.
  /// @param arg number to modify
  /// @param exp power of two to multiply with
  /// @return \a arg multplied by 2 raised to \a exp
  //    template<typename T> enable_t<half, T> scalbn(T arg, int
  //    exp) { return functions::scalbln(arg, exp); }
  inline half scalbn(half arg, int exp)
  {
    return functions::scalbln(arg, exp);
  }
  inline half scalbn(expr arg, int exp)
  {
    return functions::scalbln(arg, exp);
  }

  /// Multiply by power of two.
  /// @param arg number to modify
  /// @param exp power of two to multiply with
  /// @return \a arg multplied by 2 raised to \a exp
  //    template<typename T> enable_t<half, T> scalbln(T arg, long
  //    exp) { return functions::scalbln(arg, exp); }
  inline half scalbln(half arg, long exp)
  {
    return functions::scalbln(arg, exp);
  }
  inline half scalbln(expr arg, long exp)
  {
    return functions::scalbln(arg, exp);
  }

  /// Extract exponent.
  /// @param arg number to query
  /// @return floating point exponent
  /// @retval FP_ILOGB0 for zero
  /// @retval FP_ILOGBNAN for NaN
  /// @retval MAX_INT for infinity
  //    template<typename T> enable_t<int,T> ilogb(T arg) { return
  //    functions::ilogb(arg); }
  inline int ilogb(half arg)
  {
    return functions::ilogb(arg);
  }
  inline int ilogb(expr arg)
  {
    return functions::ilogb(arg);
  }

  /// Extract exponent.
  /// @param arg number to query
  /// @return floating point exponent
  //    template<typename T> enable_t<half, T> logb(T arg) { return
  //    functions::logb(arg); }
  inline half logb(half arg)
  {
    return functions::logb(arg);
  }
  inline half logb(expr arg)
  {
    return functions::logb(arg);
  }

  /// Next representable value.
  /// @param from value to compute next representable value for
  /// @param to direction towards which to compute next value
  /// @return next representable value after \a from in direction towards \a to
  //    template<typename T,typename U> enable_t<half,T,U>
  //    nextafter(T from, U to) { return functions::nextafter(from, to); }
  inline half nextafter(half from, half to)
  {
    return functions::nextafter(from, to);
  }
  inline half nextafter(half from, expr to)
  {
    return functions::nextafter(from, to);
  }
  inline half nextafter(expr from, half to)
  {
    return functions::nextafter(from, to);
  }
  inline half nextafter(expr from, expr to)
  {
    return functions::nextafter(from, to);
  }

  /// Next representable value.
  /// @param from value to compute next representable value for
  /// @param to direction towards which to compute next value
  /// @return next representable value after \a from in direction towards \a to
  //    template<typename T> enable_t<half, T> nexttoward(T from,
  //    long double to) { return functions::nexttoward(from, to); }
  inline half nexttoward(half from, long double to)
  {
    return functions::nexttoward(from, to);
  }
  inline half nexttoward(expr from, long double to)
  {
    return functions::nexttoward(from, to);
  }

  /// Take sign.
  /// @param x value to change sign for
  /// @param y value to take sign from
  /// @return value equal to \a x in magnitude and to \a y in sign
  //    template<typename T,typename U> enable_t<half,T,U>
  //    copysign(T x, U y) { return functions::copysign(x, y); }
  inline half copysign(half x, half y)
  {
    return functions::copysign(x, y);
  }
  inline half copysign(half x, expr y)
  {
    return functions::copysign(x, y);
  }
  inline half copysign(expr x, half y)
  {
    return functions::copysign(x, y);
  }
  inline half copysign(expr x, expr y)
  {
    return functions::copysign(x, y);
  }

  /// @}
  /// @name Floating point classification
  /// @{

  /// Classify floating point value.
  /// @param arg number to classify
  /// @retval FP_ZERO for positive and negative zero
  /// @retval FP_SUBNORMAL for subnormal numbers
  /// @retval FP_INFINITY for positive and negative infinity
  /// @retval FP_NAN for NaNs
  /// @retval FP_NORMAL for all other (normal) values
  //    template<typename T> enable_t<int,T> fpclassify(T arg) {
  //    return functions::fpclassify(arg); }
  inline int fpclassify(half arg)
  {
    return functions::fpclassify(arg);
  }

  inline int fpclassify(expr arg)
  {
    return functions::fpclassify(arg);
  }

  /// Check if finite number.
  /// @param arg number to check
  /// @retval true if neither infinity nor NaN
  /// @retval false else
  //    template<typename T> enable_t<bool,T> isfinite(T arg) {
  //    return functions::isfinite(arg); }
  inline bool isfinite(half arg)
  {
    return functions::isfinite(arg);
  }
  inline bool isfinite(expr arg)
  {
    return functions::isfinite(arg);
  }

  /// Check for infinity.
  /// @param arg number to check
  /// @retval true for positive or negative infinity
  /// @retval false else
  //    template<typename T> enable_t<bool,T> isinf(T arg) { return
  //    functions::isinf(arg); }
  inline bool isinf(half arg)
  {
    return functions::isinf(arg);
  }
  inline bool isinf(expr arg)
  {
    return functions::isinf(arg);
  }

  /// Check for NaN.
  /// @param arg number to check
  /// @retval true for NaNs
  /// @retval false else
  //    template<typename T> enable_t<bool,T> isnan(T arg) { return
  //    functions::isnan(arg); }
  inline bool isnan(half arg)
  {
    return functions::isnan(arg);
  }
  inline bool isnan(expr arg)
  {
    return functions::isnan(arg);
  }

  /// Check if normal number.
  /// @param arg number to check
  /// @retval true if normal number
  /// @retval false if either subnormal, zero, infinity or NaN
  //    template<typename T> enable_t<bool,T> isnormal(T arg) {
  //    return functions::isnormal(arg); }
  inline bool isnormal(half arg)
  {
    return functions::isnormal(arg);
  }
  inline bool isnormal(expr arg)
  {
    return functions::isnormal(arg);
  }

  /// Check sign.
  /// @param arg number to check
  /// @retval true for negative number
  /// @retval false for positive number
  //    template<typename T> enable_t<bool,T> signbit(T arg) {
  //    return functions::signbit(arg); }
  inline bool signbit(half arg)
  {
    return functions::signbit(arg);
  }
  inline bool signbit(expr arg)
  {
    return functions::signbit(arg);
  }

  /// @}
  /// @name Comparison
  /// @{

  /// Comparison for greater than.
  /// @param x first operand
  /// @param y second operand
  /// @retval true if \a x greater than \a y
  /// @retval false else
  //    template<typename T,typename U> enable_t<bool, T, U>
  //    isgreater(T x, U y) { return functions::isgreater(x, y); }
  inline bool isgreater(half x, half y)
  {
    return functions::isgreater(x, y);
  }
  inline bool isgreater(half x, expr y)
  {
    return functions::isgreater(x, y);
  }
  inline bool isgreater(expr x, half y)
  {
    return functions::isgreater(x, y);
  }
  inline bool isgreater(expr x, expr y)
  {
    return functions::isgreater(x, y);
  }

  /// Comparison for greater equal.
  /// @param x first operand
  /// @param y second operand
  /// @retval true if \a x greater equal \a y
  /// @retval false else
  //    template<typename T,typename U> enable_t<bool, T, U>
  //    isgreaterequal(T x, U y) { return functions::isgreaterequal(x, y); }
  inline bool isgreaterequal(half x, half y)
  {
    return functions::isgreaterequal(x, y);
  }
  inline bool isgreaterequal(half x, expr y)
  {
    return functions::isgreaterequal(x, y);
  }
  inline bool isgreaterequal(expr x, half y)
  {
    return functions::isgreaterequal(x, y);
  }
  inline bool isgreaterequal(expr x, expr y)
  {
    return functions::isgreaterequal(x, y);
  }

  /// Comparison for less than.
  /// @param x first operand
  /// @param y second operand
  /// @retval true if \a x less than \a y
  /// @retval false else
  //    template<typename T,typename U> enable_t<bool, T, U> isless(T
  //    x, U y) { return functions::isless(x, y); }
  inline bool isless(half x, half y)
  {
    return functions::isless(x, y);
  }
  inline bool isless(half x, expr y)
  {
    return functions::isless(x, y);
  }
  inline bool isless(expr x, half y)
  {
    return functions::isless(x, y);
  }
  inline bool isless(expr x, expr y)
  {
    return functions::isless(x, y);
  }

  /// Comparison for less equal.
  /// @param x first operand
  /// @param y second operand
  /// @retval true if \a x less equal \a y
  /// @retval false else
  //    template<typename T,typename U> enable_t<bool, T, U>
  //    islessequal(T x, U y) { return functions::islessequal(x, y); }
  inline bool islessequal(half x, half y)
  {
    return functions::islessequal(x, y);
  }
  inline bool islessequal(half x, expr y)
  {
    return functions::islessequal(x, y);
  }
  inline bool islessequal(expr x, half y)
  {
    return functions::islessequal(x, y);
  }
  inline bool islessequal(expr x, expr y)
  {
    return functions::islessequal(x, y);
  }

  /// Comarison for less or greater.
  /// @param x first operand
  /// @param y second operand
  /// @retval true if either less or greater
  /// @retval false else
  //    template<typename T,typename U> enable_t<bool, T, U>
  //    islessgreater(T x, U y) { return functions::islessgreater(x, y); }
  inline bool islessgreater(half x, half y)
  {
    return functions::islessgreater(x, y);
  }
  inline bool islessgreater(half x, expr y)
  {
    return functions::islessgreater(x, y);
  }
  inline bool islessgreater(expr x, half y)
  {
    return functions::islessgreater(x, y);
  }
  inline bool islessgreater(expr x, expr y)
  {
    return functions::islessgreater(x, y);
  }

  /// Check if unordered.
  /// @param x first operand
  /// @param y second operand
  /// @retval true if unordered (one or two NaN operands)
  /// @retval false else
  //    template<typename T,typename U> enable_t<bool, T, U>
  //    isunordered(T x, U y) { return functions::isunordered(x, y); }
  inline bool isunordered(half x, half y)
  {
    return functions::isunordered(x, y);
  }
  inline bool isunordered(half x, expr y)
  {
    return functions::isunordered(x, y);
  }
  inline bool isunordered(expr x, half y)
  {
    return functions::isunordered(x, y);
  }
  inline bool isunordered(expr x, expr y)
  {
    return functions::isunordered(x, y);
  }

  /// @name Casting
  /// @{

  /// Cast to or from half-precision floating point number.
  /// This casts between [half](\ref shift::math::half) and any built-in
  /// arithmetic type. The values are converted directly using the given
  /// rounding mode, without any roundtrip over `float` that a `static_cast`
  /// would otherwise do. It uses the default rounding mode.
  ///
  /// Using this cast with neither of the two types being a [half](\ref
  /// shift::math::half) or with any of the two types not being a built-in
  /// arithmetic type (apart from [half](\ref shift::math::half), of course)
  /// results in a compiler error and casting between [half](\ref
  /// shift::math::half)s is just a no-op.
  /// @tparam T destination type (half or built-in arithmetic type)
  /// @tparam U source type (half or built-in arithmetic type)
  /// @param arg value to cast
  /// @return \a arg converted to destination type
  template <typename T, typename U>
  T half_cast(U arg)
  {
    return half_caster<T, U>::cast(arg);
  }

  /// Cast to or from half-precision floating point number.
  /// This casts between [half](\ref shift::math::half) and any built-in
  /// arithmetic type. The values are converted directly using the given
  /// rounding mode, without any roundtrip over `float` that a `static_cast`
  /// would otherwise do.
  ///
  /// Using this cast with neither of the two types being a [half](\ref
  /// shift::math::half) or with any of the two types not being a built-in
  /// arithmetic type (apart from [half](\ref shift::math::half), of course)
  /// results in a compiler error and casting between [half](\ref
  /// shift::math::half)s is just a no-op.
  /// @tparam T destination type (half or built-in arithmetic type)
  /// @tparam R rounding mode to use.
  /// @tparam U source type (half or built-in arithmetic type)
  /// @param arg value to cast
  /// @return \a arg converted to destination type
  template <typename T, std::float_round_style R, typename U>
  T half_cast(U arg)
  {
    return half_caster<T, U, R>::cast(arg);
  }
  /// @}
}

using detail::operator==;
using detail::operator!=;
using detail::operator<;
using detail::operator>;
using detail::operator<=;
using detail::operator>=;
using detail::operator+;
using detail::operator-;
using detail::operator*;
using detail::operator/;
using detail::operator<<;
using detail::operator>>;

using detail::abs;
using detail::acos;
using detail::acosh;
using detail::asin;
using detail::asinh;
using detail::atan;
using detail::atan2;
using detail::atanh;
using detail::cbrt;
using detail::ceil;
using detail::copysign;
using detail::cos;
using detail::cosh;
using detail::erf;
using detail::erfc;
using detail::exp;
using detail::exp2;
using detail::expm1;
using detail::fabs;
using detail::fdim;
using detail::floor;
using detail::fma;
using detail::fmax;
using detail::fmin;
using detail::fmod;
using detail::fpclassify;
using detail::frexp;
using detail::hypot;
using detail::ilogb;
using detail::isfinite;
using detail::isgreater;
using detail::isgreaterequal;
using detail::isinf;
using detail::isless;
using detail::islessequal;
using detail::islessgreater;
using detail::isnan;
using detail::isnormal;
using detail::isunordered;
using detail::ldexp;
using detail::lgamma;
using detail::llrint;
using detail::llround;
using detail::log;
using detail::log10;
using detail::log1p;
using detail::log2;
using detail::logb;
using detail::lrint;
using detail::lround;
using detail::modf;
using detail::nanh;
using detail::nearbyint;
using detail::nextafter;
using detail::nexttoward;
using detail::pow;
using detail::remainder;
using detail::remquo;
using detail::rint;
using detail::round;
using detail::scalbln;
using detail::scalbn;
using detail::signbit;
using detail::sin;
using detail::sinh;
using detail::sqrt;
using detail::tan;
using detail::tanh;
using detail::tgamma;
using detail::trunc;

using detail::half_cast;
}

/// Extensions to the C++ standard library.
namespace std
{
/// Numeric limits for half-precision floats.
/// Because of the underlying single-precision implementation of many
/// operations, it inherits some properties from `std::numeric_limits<float>`.
template <>
class numeric_limits<shift::math::half> : public numeric_limits<float>
{
public:
  /// Supports signed values.
  static constexpr bool is_signed = true;

  /// Is not exact.
  static constexpr bool is_exact = false;

  /// Doesn't provide modulo arithmetic.
  static constexpr bool is_modulo = false;

  /// IEEE conformant.
  static constexpr bool is_iec559 = true;

  /// Supports infinity.
  static constexpr bool has_infinity = true;

  /// Supports quiet NaNs.
  static constexpr bool has_quiet_NaN = true;

  /// Supports subnormal values.
  static constexpr float_denorm_style has_denorm = denorm_present;

  /// Rounding mode.
  /// Due to the mix of internal single-precision computations (using the
  /// rounding mode of the underlying single-precision implementation) with the
  /// rounding mode of the single-to-half conversions, the actual rounding mode
  /// might be `std::round_indeterminate` if the default half-precision rounding
  /// mode doesn't match the single-precision rounding mode.
  static constexpr float_round_style round_style =
    (std::numeric_limits<float>::round_style ==
     shift::math::half_config::round_style)
      ? shift::math::half_config::round_style
      : round_indeterminate;

  /// Significant digits.
  static constexpr int digits = 11;

  /// Significant decimal digits.
  static constexpr int digits10 = 3;

  /// Required decimal digits to represent all possible values.
  static constexpr int max_digits10 = 5;

  /// Number base.
  static constexpr int radix = 2;

  /// One more than smallest exponent.
  static constexpr int min_exponent = -13;

  /// Smallest normalized representable power of 10.
  static constexpr int min_exponent10 = -4;

  /// One more than largest exponent
  static constexpr int max_exponent = 16;

  /// Largest finitely representable power of 10.
  static constexpr int max_exponent10 = 4;

  /// Smallest positive normal value.
  static constexpr shift::math::half min() noexcept
  {
    return shift::math::half(shift::math::detail::binary, 0x0400);
  }

  /// Smallest finite value.
  static constexpr shift::math::half lowest() noexcept
  {
    return shift::math::half(shift::math::detail::binary, 0xFBFF);
  }

  /// Largest finite value.
  static constexpr shift::math::half max() noexcept
  {
    return shift::math::half(shift::math::detail::binary, 0x7BFF);
  }

  /// Difference between one and next representable value.
  static constexpr shift::math::half epsilon() noexcept
  {
    return shift::math::half(shift::math::detail::binary, 0x1400);
  }

  /// Maximum rounding error.
  static constexpr shift::math::half round_error() noexcept
  {
    return shift::math::half(
      shift::math::detail::binary,
      (round_style == std::round_to_nearest) ? 0x3800 : 0x3C00);
  }

  /// Positive infinity.
  static constexpr shift::math::half infinity() noexcept
  {
    return shift::math::half(shift::math::detail::binary, 0x7C00);
  }

  /// Quiet NaN.
  static constexpr shift::math::half quiet_NaN() noexcept
  {
    return shift::math::half(shift::math::detail::binary, 0x7FFF);
  }

  /// Signalling NaN.
  static constexpr shift::math::half signaling_NaN() noexcept
  {
    return shift::math::half(shift::math::detail::binary, 0x7DFF);
  }

  /// Smallest positive subnormal value.
  static constexpr shift::math::half denorm_min() noexcept
  {
    return shift::math::half(shift::math::detail::binary, 0x0001);
  }
};

/// Hash function for half-precision floats.
/// This is only defined if C++11 `std::hash` is supported and enabled.
template <>
struct hash<shift::math::half>  //: unary_function<shift::math::half,size_t>
{
  /// Type of function argument.
  using argument_type = shift::math::half;

  /// Function return type.
  using result_type = size_t;

  /// Compute hash function.
  /// @param arg half to hash
  /// @return hash value
  result_type operator()(argument_type arg) const
  {
    return hash<std::uint16_t>()(static_cast<unsigned>(arg._data) &
                                 -(arg._data != 0x8000));
  }
};
}

#endif
