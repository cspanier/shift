#ifndef SHIFT_MATH_DETAIL_MATRIX_STORAGE_H
#define SHIFT_MATH_DETAIL_MATRIX_STORAGE_H

#include <cstdint>
#include <array>
#include <shift/platform/assert.h>
#include <shift/core/types.h>
#include "shift/math/utility.h"

namespace shift::math::detail
{
/// A generic vector data storage structure offering union .
template <std::size_t Rows, std::size_t Columns, typename T>
struct matrix_storage
{
  /// Default constructor not initializing anything.
  constexpr explicit matrix_storage() noexcept = default;

  /// Copy constructor.
  constexpr matrix_storage(const matrix_storage& /*other*/) = default;

  /// Move constructor.
  constexpr matrix_storage(matrix_storage&& /*other*/) noexcept = default;

  /// Constructor supporting brace-init-lists.
  template <typename... Ts>
  constexpr matrix_storage(Ts... args) noexcept : array{args...}
  {
  }

  /// Constructor initializing data from an array of values.
  constexpr explicit matrix_storage(std::array<T, Columns * Rows> args) noexcept
  : array(args)
  {
  }

  /// Copy assignment operator.
  constexpr matrix_storage& operator=(
    const matrix_storage& /*other*/) noexcept = default;

  /// Move assignment operator.
  constexpr matrix_storage& operator=(matrix_storage&& /*other*/) noexcept =
    default;

  union {
    std::array<T, Columns * Rows> array;
  };
};

/// A generic vector data storage structure offering union .
template <std::size_t Rows, typename T>
struct matrix_storage<Rows, 1, T>
{
  /// Default constructor not initializing anything.
  constexpr explicit matrix_storage() noexcept = default;

  /// Copy constructor.
  constexpr matrix_storage(const matrix_storage& /*other*/) = default;

  /// Move constructor.
  constexpr matrix_storage(matrix_storage&& /*other*/) noexcept = default;

  /// Constructor supporting brace-init-lists.
  template <typename... Ts>
  constexpr matrix_storage(Ts... args) noexcept : array{args...}
  {
  }

  /// Constructor initializing data from an array of values.
  constexpr explicit matrix_storage(std::array<T, Rows> args) noexcept
  : array(args)
  {
  }

  /// Copy assignment operator.
  constexpr matrix_storage& operator=(
    const matrix_storage& /*other*/) noexcept = default;

  /// Move assignment operator.
  constexpr matrix_storage& operator=(matrix_storage&& /*other*/) noexcept =
    default;

  union {
    std::array<T, Rows> array;
  };
};

template <typename T>
struct matrix_storage<1, 1, T>
{
  /// Default constructor not initializing anything.
  constexpr explicit matrix_storage() noexcept = default;

  /// Copy constructor.
  constexpr matrix_storage(const matrix_storage& /*other*/) = default;

  /// Move constructor.
  constexpr matrix_storage(matrix_storage&& /*other*/) noexcept = default;

  /// Constructor supporting brace-init-lists.
  template <typename... Ts>
  constexpr matrix_storage(Ts... args) noexcept : array{args...}
  {
  }

  /// Constructor initializing data from an array of values.
  constexpr explicit matrix_storage(std::array<T, 1> args) noexcept
  : array(args)
  {
  }

  /// Copy assignment operator.
  constexpr matrix_storage& operator=(
    const matrix_storage& /*other*/) noexcept = default;

  /// Move assignment operator.
  constexpr matrix_storage& operator=(matrix_storage&& /*other*/) noexcept =
    default;

  union {
    std::array<T, 1> array;
    T x;
    T r;
    T left;
    T width;
    T phi;
  };
};

template <typename T>
struct matrix_storage<2, 1, T>
{
  /// Default constructor not initializing anything.
  constexpr explicit matrix_storage() noexcept = default;

  /// Copy constructor.
  constexpr matrix_storage(const matrix_storage& /*other*/) = default;

  /// Move constructor.
  constexpr matrix_storage(matrix_storage&& /*other*/) noexcept = default;

  /// Constructor supporting brace-init-lists.
  template <typename... Ts>
  constexpr matrix_storage(Ts... args) noexcept : array{args...}
  {
  }

  /// Constructor initializing data from an array of values.
  constexpr explicit matrix_storage(std::array<T, 2> args) noexcept
  : array(args)
  {
  }

  /// Copy assignment operator.
  constexpr matrix_storage& operator=(
    const matrix_storage& /*other*/) noexcept = default;

  /// Move assignment operator.
  constexpr matrix_storage& operator=(matrix_storage&& /*other*/) noexcept =
    default;

  union {
    std::array<T, 2> array;
    struct
    {
      T x;
      T y;
    };
    struct
    {
      T r;
      T g;
    };
    struct
    {
      T left;
      T top;
    };
    struct
    {
      T width;
      T height;
    };
    struct
    {
      T phi;
      T theta;
    };
    struct
    {
      T longitude;
      T latitude;
    };
  };
};

template <typename T>
struct matrix_storage<3, 1, T>
{
  /// Default constructor not initializing anything.
  constexpr explicit matrix_storage() noexcept = default;

  /// Copy constructor.
  constexpr matrix_storage(const matrix_storage& /*other*/) = default;

  /// Move constructor.
  constexpr matrix_storage(matrix_storage&& /*other*/) noexcept = default;

  /// Constructor supporting brace-init-lists.
  template <typename... Ts>
  constexpr matrix_storage(Ts... args) noexcept : array{args...}
  {
  }

  /// Constructor initializing data from an array of values.
  constexpr explicit matrix_storage(std::array<T, 3> args) noexcept
  : array(args)
  {
  }

  /// Copy assignment operator.
  constexpr matrix_storage& operator=(
    const matrix_storage& /*other*/) noexcept = default;

  /// Move assignment operator.
  constexpr matrix_storage& operator=(matrix_storage&& /*other*/) noexcept =
    default;

  union {
    std::array<T, 3> array;
    struct
    {
      T x;
      T y;
      T z;
    };
    struct
    {
      T width;
      T height;
      T depth;
    };
    struct
    {
      T r;
      T g;
      T b;
    };
    struct
    {
      T phi;
      T theta;
      T psi;
    };
    struct
    {
      T longitude;
      T latitude;
      T elevation;
    };
  };
};

template <typename T>
struct matrix_storage<4, 1, T>
{
  /// Default constructor not initializing anything.
  constexpr explicit matrix_storage() noexcept = default;

  /// Copy constructor.
  constexpr matrix_storage(const matrix_storage& /*other*/) = default;

  /// Move constructor.
  constexpr matrix_storage(matrix_storage&& /*other*/) noexcept = default;

  /// Constructor supporting brace-init-lists.
  template <typename... Ts>
  constexpr matrix_storage(Ts... args) noexcept : array{args...}
  {
  }

  /// Constructor initializing data from an array of values.
  constexpr explicit matrix_storage(std::array<T, 4> args) noexcept
  : array(args)
  {
  }

  /// Copy assignment operator.
  constexpr matrix_storage& operator=(
    const matrix_storage& /*other*/) noexcept = default;

  /// Move assignment operator.
  constexpr matrix_storage& operator=(matrix_storage&& /*other*/) noexcept =
    default;

  union {
    std::array<T, 4> array;
    struct
    {
      T x;
      T y;
      T z;
      T w;
    };
    struct
    {
      T r;
      T g;
      T b;
      T a;
    };
    struct
    {
      T left;
      T top;
      union {
        T right;
        T width;
      };
      union {
        T bottom;
        T height;
      };
    };
  };
};
}

#endif
