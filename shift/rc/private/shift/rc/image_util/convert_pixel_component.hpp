#ifndef SHIFT_RC_IMAGE_UTIL_CONVERT_PIXEL_COMPONENT_HPP
#define SHIFT_RC_IMAGE_UTIL_CONVERT_PIXEL_COMPONENT_HPP

#include <cstdint>
#include <type_traits>
#include <limits>
#include <shift/core/algorithm.hpp>

namespace shift::rc::image_util::detail
{
/// A support type to convert unsigned integer pixel components to different
/// unsigned integer types.
template <typename Destination>
struct convert_uint_pixel_component;

template <>
struct convert_uint_pixel_component<std::uint8_t>
{
  std::uint8_t operator()(std::uint8_t source)
  {
    return source;
  }

  std::uint8_t operator()(std::uint16_t source)
  {
    return static_cast<std::uint8_t>(source >> 8);
  }

  std::uint8_t operator()(std::uint32_t source)
  {
    return static_cast<std::uint8_t>(source >> 24);
  }
};

template <>
struct convert_uint_pixel_component<std::uint16_t>
{
  std::uint16_t operator()(std::uint8_t source)
  {
    auto temp = static_cast<std::uint16_t>(source);
    return static_cast<std::uint16_t>(temp << 8 | temp);
  }

  std::uint16_t operator()(std::uint16_t source)
  {
    return source;
  }

  std::uint16_t operator()(std::uint32_t source)
  {
    return static_cast<std::uint16_t>(source >> 16);
  }
};

template <>
struct convert_uint_pixel_component<std::uint32_t>
{
  std::uint32_t operator()(std::uint8_t source)
  {
    auto temp = static_cast<std::uint16_t>(source);
    return static_cast<std::uint32_t>(temp << 24 | temp << 16 | temp << 8 |
                                      temp);
  }

  std::uint32_t operator()(std::uint16_t source)
  {
    auto temp = static_cast<std::uint32_t>(source);
    return static_cast<std::uint32_t>(temp << 16 | temp);
  }

  std::uint32_t operator()(std::uint32_t source)
  {
    return source;
  }
};

/// A support type to convert signed integer pixel components to different
/// signed integer types.
template <typename Destination>
struct convert_int_pixel_component;

template <>
struct convert_int_pixel_component<std::int8_t>
{
  std::int8_t operator()(std::int8_t source)
  {
    return source;
  }

  std::int8_t operator()(std::int16_t source)
  {
    return static_cast<std::int8_t>(source >> 8);
  }

  std::int8_t operator()(std::int32_t source)
  {
    return static_cast<std::int8_t>(source >> 24);
  }
};

template <>
struct convert_int_pixel_component<std::int16_t>
{
  std::int16_t operator()(std::int8_t source)
  {
    auto temp = static_cast<std::uint16_t>(source);
    return static_cast<std::int16_t>(temp << 8 | temp);
  }

  std::int16_t operator()(std::int16_t source)
  {
    return source;
  }

  std::int16_t operator()(std::int32_t source)
  {
    return static_cast<std::int16_t>(source >> 16);
  }
};

template <>
struct convert_int_pixel_component<std::int32_t>
{
  std::int32_t operator()(std::int8_t source)
  {
    auto temp = static_cast<std::uint32_t>(source) << 24;
    return static_cast<std::int32_t>(temp << 24 | temp << 16 | temp << 8 |
                                     temp);
  }

  std::int32_t operator()(std::int16_t source)
  {
    auto temp = static_cast<std::uint32_t>(source);
    return static_cast<std::int32_t>(temp << 16 | temp);
  }

  std::int32_t operator()(std::int32_t source)
  {
    return source;
  }
};

/// A support type to convert signed integer types to unsigned integer types and
/// vise versa.
struct convert_pixel_component_sign
{
  std::uint8_t operator()(std::int8_t source)
  {
    return static_cast<std::uint8_t>(source) + 0x80;
  }

  std::uint16_t operator()(std::int16_t source)
  {
    return static_cast<std::uint16_t>(source) + 0x8000;
  }

  std::uint32_t operator()(std::int32_t source)
  {
    return static_cast<std::uint32_t>(source) + 0x80000000u;
  }

  std::int8_t operator()(std::uint8_t source)
  {
    return static_cast<std::int8_t>(source + 0x80);
  }

  std::int16_t operator()(std::uint16_t source)
  {
    return static_cast<std::int16_t>(source + 0x8000);
  }

  std::int32_t operator()(std::uint32_t source)
  {
    return static_cast<std::int32_t>(source + 0x80000000u);
  }
};

/// A support type to convert a single pixel component from one type to another.
/// @remarks
///   In most cases a single static_cast would not be enough because of
///   different source ranges of source and destination types and rounding
///   problems.
template <typename Destination, typename Source>
struct convert_pixel_component
{
  Destination operator()(Source source)
  {
    if constexpr (std::is_integral_v<Destination> &&
                  std::is_unsigned_v<Destination>)
    {
      if constexpr (std::is_integral_v<Source> && std::is_unsigned_v<Source>)
        return convert_uint_pixel_component<Destination>{}(source);
      else if constexpr (std::is_integral_v<Source> && std::is_signed_v<Source>)
      {
        auto temp = convert_pixel_component_sign{}(source);
        return convert_uint_pixel_component<Destination>{}(temp);
      }
      else if constexpr (std::is_floating_point_v<Source>)
      {
        return static_cast<Destination>(
          core::clamp(source, Source{0}, Source{1}) *
          std::numeric_limits<Destination>::max());
      }
    }
    else if constexpr (std::is_integral_v<Destination> &&
                       std::is_signed_v<Destination>)
    {
      if constexpr (std::is_integral_v<Source> && std::is_unsigned_v<Source>)
      {
        auto temp = convert_pixel_component_sign{}(source);
        return convert_int_pixel_component<Destination>{}(temp);
      }
      else if constexpr (std::is_integral_v<Source> && std::is_signed_v<Source>)
        return convert_int_pixel_component<Destination>{}(source);
      else if constexpr (std::is_floating_point_v<Source>)
      {
        source = core::clamp(source, Source{-1}, Source{1});
        if (source >= 0)
        {
          return static_cast<Destination>(
            source * std::numeric_limits<Destination>::max());
        }
        else
        {
          return static_cast<Destination>(
            -source * std::numeric_limits<Destination>::min());
        }
      }
    }
    else if constexpr (std::is_floating_point_v<Destination>)
    {
      if constexpr (std::is_integral_v<Source> && std::is_unsigned_v<Source>)
      {
        return static_cast<Destination>(source) /
               std::numeric_limits<Source>::max();
      }
      else if constexpr (std::is_integral_v<Source> && std::is_signed_v<Source>)
      {
        auto destination = static_cast<Destination>(source);
        if (source >= 0)
          return destination / std::numeric_limits<Source>::max();
        else
          return -destination / std::numeric_limits<Source>::min();
      }
      else if constexpr (std::is_floating_point_v<Source>)
        return static_cast<Destination>(source);
    }

    BOOST_ASSERT(false);
    return static_cast<Destination>(source);
  }
};
}

#endif
