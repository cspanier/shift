#ifndef SHIFT_CORE_BITFIELD_H
#define SHIFT_CORE_BITFIELD_H

#include <type_traits>
#include "shift/core/core.h"
#include "shift/core/types.h"

namespace shift::core
{
/// A type safe bitfield which only allows interaction with a strongly typed
/// enum.
template <typename TEnum,
          typename TStorage = typename std::underlying_type_t<TEnum>>
struct bit_field
{
  using storage_type = TStorage;
  using flag_type = TEnum;

  /// Default constructor.
  /// @remarks
  ///   The default constructor is required to make bit_field a trivial type.
  ///   Note that we can neither implement a custom default constructor nor
  ///   assign _bits a default value because this would make this type
  ///   non-trivial.
  constexpr bit_field() = default;

  /// Constructor.
  constexpr bit_field(TEnum bit) : _bits(static_cast<storage_type>(bit))
  {
    static_assert(std::is_trivial_v<bit_field>);
  }

  /// Constructor.
  explicit constexpr bit_field(storage_type bits) : _bits(bits)
  {
    static_assert(std::is_trivial_v<bit_field>);
  }

  /// Copy constructor.
  constexpr bit_field(const bit_field&) = default;

  /// Move constructor.
  constexpr bit_field(bit_field&&) = default;

  /// Copy assignment operator.
  constexpr bit_field& operator=(const bit_field&) = default;

  /// Move assignment operator.
  constexpr bit_field& operator=(bit_field&&) = default;

  /// Conversion operator to the underlying type of type TEnum, which returns
  /// the bit field.
  explicit constexpr operator storage_type() const
  {
    return _bits;
  }

  /// Conversion operator to type bool which checks whether any bits are set.
  constexpr operator bool() const
  {
    return _bits != 0;
  }

  ///
  constexpr bool operator==(const bit_field& other) const noexcept
  {
    return _bits == other._bits;
  }

  ///
  constexpr bool operator!=(const bit_field& other) const noexcept
  {
    return !(*this == other);
  }

  ///
  constexpr bit_field operator!() const
  {
    return bit_field(!_bits);
  }

  ///
  constexpr bit_field operator~() const
  {
    return bit_field(~_bits);
  }

  ///
  constexpr bit_field& operator&=(TEnum bit)
  {
    _bits &= static_cast<storage_type>(bit);
    return *this;
  }

  ///
  constexpr bit_field& operator|=(TEnum bit)
  {
    _bits |= static_cast<storage_type>(bit);
    return *this;
  }

  ///
  constexpr bit_field& operator^=(TEnum bit)
  {
    _bits ^= static_cast<storage_type>(bit);
    return *this;
  }

  ///
  constexpr bit_field& operator&=(const bit_field bits)
  {
    _bits &= bits._bits;
    return *this;
  }

  ///
  constexpr bit_field& operator|=(const bit_field bits)
  {
    _bits |= bits._bits;
    return *this;
  }

  ///
  constexpr bit_field& operator^=(const bit_field bits)
  {
    _bits ^= bits._bits;
    return *this;
  }

  ///
  constexpr bit_field& set(TEnum bit)
  {
    _bits |= static_cast<storage_type>(bit);
    return *this;
  }

  ///
  constexpr bit_field& set(const bit_field bits)
  {
    _bits |= bits._bits;
    return *this;
  }

  ///
  constexpr bit_field& reset(TEnum bit)
  {
    _bits &= ~static_cast<storage_type>(bit);
    return *this;
  }

  ///
  constexpr bit_field& reset(const bit_field bits)
  {
    _bits &= ~bits._bits;
    return *this;
  }

  ///
  constexpr bool test(TEnum bit) const
  {
    return (_bits & static_cast<storage_type>(bit)) != 0;
  }

  ///
  constexpr bool test(const bit_field bits) const
  {
    return (_bits & bits._bits) != 0;
  }

  /// Grants access to the underlying storage.
  storage_type& data()
  {
    return _bits;
  }

  /// Grants access to the underlying storage.
  const storage_type& data() const
  {
    return _bits;
  }

  /// Clears all bits.
  void clear()
  {
    _bits = 0;
  }

  ///
  friend constexpr bit_field operator&(bit_field lhs, bit_field rhs)
  {
    return bit_field(static_cast<storage_type>(lhs) &
                     static_cast<storage_type>(rhs));
  }

  ///
  friend constexpr bit_field operator|(bit_field lhs, bit_field rhs)
  {
    return bit_field(static_cast<storage_type>(lhs) |
                     static_cast<storage_type>(rhs));
  }

  ///
  friend constexpr bit_field operator^(bit_field lhs, bit_field rhs)
  {
    return bit_field(static_cast<storage_type>(lhs) ^
                     static_cast<storage_type>(rhs));
  }

  ///
  friend constexpr bit_field operator&(bit_field lhs, TEnum rhs)
  {
    return bit_field(static_cast<storage_type>(lhs) &
                     static_cast<storage_type>(rhs));
  }

  ///
  friend constexpr bit_field operator|(bit_field lhs, TEnum rhs)
  {
    return bit_field(static_cast<storage_type>(lhs) |
                     static_cast<storage_type>(rhs));
  }

  ///
  friend constexpr bit_field operator^(bit_field lhs, TEnum rhs)
  {
    return bit_field(static_cast<storage_type>(lhs) ^
                     static_cast<storage_type>(rhs));
  }

  ///
  friend constexpr bit_field operator&(TEnum lhs, bit_field rhs)
  {
    return bit_field(static_cast<storage_type>(lhs) &
                     static_cast<storage_type>(rhs));
  }

  ///
  friend constexpr bit_field operator|(TEnum lhs, bit_field rhs)
  {
    return bit_field(static_cast<storage_type>(lhs) |
                     static_cast<storage_type>(rhs));
  }

  ///
  friend constexpr bit_field operator^(TEnum lhs, bit_field rhs)
  {
    return bit_field(static_cast<storage_type>(lhs) ^
                     static_cast<storage_type>(rhs));
  }

private:
  storage_type _bits;
};
}

#endif
