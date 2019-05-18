#ifndef SHIFT_RC_IMAGE_UTIL_CONVERT_HPP
#define SHIFT_RC_IMAGE_UTIL_CONVERT_HPP

#include <cstdint>
#include <type_traits>
#include <shift/core/mpl.hpp>
#include <shift/math/half.hpp>
#include <shift/math/vector.hpp>
#include "shift/rc/image_util/linear_image_view.hpp"
#include "shift/rc/image_util/convert_pixel_component.hpp"

namespace shift::rc::image_util
{
namespace detail
{
  /// This type helps converting pixel channels from sRGB to linear and vice
  /// versa.
  template <color_space_t DestinationColorSpace, color_space_t SourceColorSpace>
  struct color_space_converter;

  template <>
  struct color_space_converter<color_space_t::unorm, color_space_t::srgb>
  {
    float operator()(float source)
    {
      if (source <= 0.0f)
        return 0.0f;
      else if (source < 0.04045f)
        return source / 12.92f;
      else if (source < 1.0f)
        return std::pow((source + 0.055f) / 1.055f, 2.4f);
      else
        return 1.0f;
    }
  };

  template <>
  struct color_space_converter<color_space_t::srgb, color_space_t::unorm>
  {
    float operator()(float source)
    {
      if (source <= 0.0f)
        return 0.0f;
      else if (source < 0.0031308f)
        return source * 12.92f;
      else if (source < 1.0f)
        return std::pow(source, 1.0f / 2.4f) * 1.055f - 0.055f;
      else
        return 1.0f;
    }
  };

  template <>
  struct color_space_converter<color_space_t::unorm, color_space_t::snorm>
  {
    float operator()(float source)
    {
      if (source <= 0.0f)
        return 0.0f;
      else if (source < 1.0f)
        return std::fma(source, 2.0f, -1.0f);
      else
        return 1.0f;
    }
  };

  template <>
  struct color_space_converter<color_space_t::snorm, color_space_t::unorm>
  {
    float operator()(float source)
    {
      if (source <= -1.0f)
        return 0.0f;
      else if (source < 1.0f)
        return std::fma(source, 0.5f, 0.5f);
      else
        return 1.0f;
    }
  };

  template <>
  struct color_space_converter<color_space_t::unorm, color_space_t::sfloat>
  {
    float operator()(float source)
    {
      if (source <= 0.0f)
        return 0.0f;
      else if (source < 1.0f)
        return std::fma(source, 2.0f, -1.0f);
      else
        return 1.0f;
    }
  };

  template <>
  struct color_space_converter<color_space_t::sfloat, color_space_t::unorm>
  {
    float operator()(float source)
    {
      if (source <= -1.0f)
        return 0.0f;
      else if (source < 1.0f)
        return std::fma(source, 0.5f, 0.5f);
      else
        return 1.0f;
    }
  };

  template <>
  struct color_space_converter<color_space_t::unorm, color_space_t::ufloat>
  {
    float operator()(float source)
    {
      return source;
    }
  };

  template <>
  struct color_space_converter<color_space_t::ufloat, color_space_t::unorm>
  {
    float operator()(float source)
    {
      return source;
    }
  };

  template <>
  struct color_space_converter<color_space_t::snorm, color_space_t::sfloat>
  {
    float operator()(float source)
    {
      return source;
    }
  };

  template <>
  struct color_space_converter<color_space_t::sfloat, color_space_t::snorm>
  {
    float operator()(float source)
    {
      return source;
    }
  };

  template <>
  struct color_space_converter<color_space_t::snorm, color_space_t::srgb>
  {
    float operator()(float source)
    {
      /// ToDo: snorm <-> srgb conversion doesn't make any sense.
      return source;
    }
  };

  template <>
  struct color_space_converter<color_space_t::srgb, color_space_t::snorm>
  {
    float operator()(float source)
    {
      /// ToDo: snorm <-> srgb conversion doesn't make any sense.
      return source;
    }
  };

  template <typename DestinationPixel, typename SourcePixel>
  struct channel_converter
  {
    template <typename DestinationChannel>
    void operator()(const DestinationChannel*,
                    DestinationPixel& destination_pixel,
                    const SourcePixel& source_pixel) const
    {
      using destination_t = typename DestinationPixel::component_t;
      using source_t = typename SourcePixel::component_t;

      constexpr auto destination_channel_index =
        channel_index_v<typename DestinationPixel::channels_t,
                        DestinationChannel>;
      static_assert(destination_channel_index <
                    DestinationPixel::channel_count);

      constexpr auto source_channel_index =
        channel_index_v<typename SourcePixel::channels_t, DestinationChannel>;

      if constexpr (source_channel_index >= SourcePixel::channel_count)
      {
        // There is no matching source channel. We have to fill the destination
        // with a default value.
        destination_pixel[destination_channel_index] = destination_t{};
      }
      else if constexpr (DestinationPixel::color_space ==
                         SourcePixel::color_space)
      {
        if constexpr (std::is_same_v<source_t, destination_t>)
        {
          // Source and destination channels are of the same type and color
          // space, so we can perform a direct copy.
          destination_pixel[destination_channel_index] =
            source_pixel[source_channel_index];
        }
        else
        {
          // Source and destination channels are of different type but in the
          // same color space.
          destination_pixel[destination_channel_index] =
            convert_pixel_component<destination_t, source_t>{}(
              source_pixel[source_channel_index]);
        }
      }
      else
      {
        // Source and destination channels are both of different type and in
        // different color spaces.
        using temp_t =
          std::conditional_t<(sizeof(source_t) < sizeof(std::uint32_t)), float,
                             double>;
        auto temp_data = convert_pixel_component<temp_t, source_t>{}(
          source_pixel[source_channel_index]);
        /// ToDo: Solve color space conversion issues...
        // temp_data =
        //   color_space_converter<DestinationPixel::color_space,
        //                         SourcePixel::color_space>{}(temp_data);
        destination_pixel[destination_channel_index] =
          convert_pixel_component<destination_t, float>{}(temp_data);
      }
    }
  };
}

/// This class converts a single pixel from one format to another.
template <typename DestinationPixel, typename SourcePixel>
class pixel_converter
{
public:
  using destination_pixel_t = DestinationPixel;
  using source_pixel_t = SourcePixel;

  void operator()(destination_pixel_t& destination,
                  const source_pixel_t& source)
  {
    if constexpr (std::is_same_v<destination_pixel_t, source_pixel_t>)
      std::memcpy(&destination, &source, destination_pixel_t::size_in_bytes);
    else
    {
      // Convert each destination pixel channel separately.
      core::for_each<typename DestinationPixel::channels_t>(
        detail::channel_converter<DestinationPixel, SourcePixel>{}, destination,
        source);
    }
  }
};
}

#endif
