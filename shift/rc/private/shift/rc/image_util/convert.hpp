#ifndef SHIFT_RC_IMAGE_UTIL_CONVERT_HPP
#define SHIFT_RC_IMAGE_UTIL_CONVERT_HPP

#include <cstdint>
#include <type_traits>
#include <boost/assert.hpp>
#include <shift/core/mpl.hpp>
#include <shift/math/half.hpp>
#include <shift/math/vector.hpp>
#include "shift/rc/image_util/linear_image_view.hpp"
#include "shift/rc/image_util/convert_pixel_component.hpp"

namespace shift::rc::image_util
{
namespace detail
{
  template <typename T, typename PixelChannel>
  struct unorm_converter
  {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>);
    static_assert(!PixelChannel::is_block_format);

    static constexpr float to_float(T value)
    {
      return value / static_cast<float>((1 << PixelChannel::size_in_bits) - 1);
    }

    static constexpr T from_float(float value)
    {
      return static_cast<T>(value * ((1 << PixelChannel::size_in_bits) - 1));
    }
  };

  template <typename T, typename PixelChannel>
  struct snorm_converter
  {
    static_assert(std::is_integral_v<T> && std::is_signed_v<T>);
    static_assert(!PixelChannel::is_block_format);

    static constexpr float to_float(T value)
    {
      static_assert(
        static_cast<T>(std::numeric_limits<std::make_unsigned_t<T>>::max()
                       << 1) == -2);
      // The minimum value of a PixelChannel::size_in_bits sized signed integer,
      // sign extended and stored in a value of type T.
      constexpr T min_value =
        static_cast<T>(std::numeric_limits<std::make_unsigned_t<T>>::max()
                       << (PixelChannel::size_in_bits - 1));
      if (value == min_value)
      {
        return -1.0f;
      }
      else
      {
        return value /
               static_cast<float>((1 << (PixelChannel::size_in_bits - 1)) - 1);
      }
    }

    static constexpr T from_float(float value)
    {
      return static_cast<T>(value *
                            ((1 << (PixelChannel::size_in_bits - 1)) - 1));
    }
  };

  struct srgb_converter
  {
    static constexpr float to_linear(float srgb_value)
    {
      if (srgb_value <= 0.0f)
        return 0.0f;
      else if (srgb_value < 0.04045f)
        return srgb_value / 12.92f;
      else if (srgb_value < 1.0f)
        return std::pow((srgb_value + 0.055f) / 1.055f, 2.4f);
      else
        return 1.0f;
    }

    static constexpr float from_linear(float linear_value)
    {
      if (linear_value <= 0.0f)
        return 0.0f;
      else if (linear_value < 0.0031308f)
        return linear_value * 12.92f;
      else if (linear_value < 1.0f)
        return std::pow(linear_value, 1.0f / 2.4f) * 1.055f - 0.055f;
      else
        return 1.0f;
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
      using destination_channel_t =
        core::get_type_opt_t<destination_channel_index,
                             typename DestinationPixel::channels_t>;

      constexpr auto source_channel_index =
        channel_index_v<typename SourcePixel::channels_t, DestinationChannel>;
      using source_channel_t =
        core::get_type_opt_t<source_channel_index,
                             typename SourcePixel::channels_t>;

      if constexpr (source_channel_index >= SourcePixel::channel_count)
      {
        // There is no matching source channel. We have to fill the destination
        // with a default value.
        destination_pixel[destination_channel_index] = destination_t{};
      }
      else if constexpr ((SourcePixel::data_type ==
                          DestinationPixel::data_type) &&
                         std::is_same_v<source_t, destination_t> &&
                         std::is_same_v<source_channel_t,
                                        destination_channel_t>)
      {
        // Source and destination channels are of the same channel type and
        // data type, so we can perform a direct copy.
        destination_pixel[destination_channel_index] =
          source_pixel[source_channel_index];
      }
      else if constexpr (SourcePixel::data_type == pixel_data_type::unorm)
      {
        if constexpr (DestinationPixel::data_type == pixel_data_type::unorm)
        {
          // Convert unorm to float.
          auto temp = unorm_converter<source_t, source_channel_t>::to_float(
            source_pixel[source_channel_index]);
          // Convert float to unorm.
          destination_pixel[destination_channel_index] =
            unorm_converter<destination_t, destination_channel_t>::from_float(
              temp);
        }
        else if constexpr (DestinationPixel::data_type ==
                           pixel_data_type::snorm)
        {
          // Convert unorm to float.
          auto temp = unorm_converter<source_t, source_channel_t>::to_float(
            source_pixel[source_channel_index]);
          // Convert float to snorm.
          destination_pixel[destination_channel_index] =
            snorm_converter<destination_t, destination_channel_t>::from_float(
              temp);
        }
        else if constexpr (DestinationPixel::data_type ==
                             pixel_data_type::ufloat ||
                           DestinationPixel::data_type ==
                             pixel_data_type::sfloat)
        {
          // Convert unorm to float.
          destination_pixel[destination_channel_index] =
            unorm_converter<source_t, source_channel_t>::to_float(
              source_pixel[source_channel_index]);
        }
        else if constexpr (DestinationPixel::data_type == pixel_data_type::srgb)
        {
          // Convert unorm to float.
          auto temp = unorm_converter<source_t, source_channel_t>::to_float(
            source_pixel[source_channel_index]);
          // Convert color space from linear to sRGB.
          temp = srgb_converter::from_linear(temp);
          // Convert float to unorm.
          destination_pixel[destination_channel_index] =
            unorm_converter<destination_t, destination_channel_t>::from_float(
              temp);
        }
        else
          BOOST_ASSERT(false);
      }
      else if constexpr (SourcePixel::data_type == pixel_data_type::snorm)
      {
        if constexpr (DestinationPixel::data_type == pixel_data_type::unorm)
        {
          // Convert snorm to float.
          auto temp = snorm_converter<source_t, source_channel_t>::to_float(
            source_pixel[source_channel_index]);
          // Clamp values < 0.0f.
          temp = std::max(temp, 0.0f);
          // Convert float to unorm.
          destination_pixel[destination_channel_index] =
            unorm_converter<destination_t, destination_channel_t>::from_float(
              temp);
        }
        else if constexpr (DestinationPixel::data_type ==
                           pixel_data_type::snorm)
        {
          // Convert snorm to float.
          auto temp = snorm_converter<source_t, source_channel_t>::to_float(
            source_pixel[source_channel_index]);
          // Convert float to snorm.
          destination_pixel[destination_channel_index] =
            snorm_converter<destination_t, destination_channel_t>::from_float(
              temp);
        }
        else if constexpr (DestinationPixel::data_type ==
                             pixel_data_type::ufloat ||
                           DestinationPixel::data_type ==
                             pixel_data_type::sfloat)
        {
          // Convert snorm -> float.
          destination_pixel[destination_channel_index] =
            snorm_converter<source_t, source_channel_t>::to_float(
              source_pixel[source_channel_index]);
        }
        else if constexpr (DestinationPixel::data_type == pixel_data_type::srgb)
        {
          // Convert snorm -> float.
          auto temp = snorm_converter<source_t, source_channel_t>::to_float(
            source_pixel[source_channel_index]);
          // Clamp values < 0.0f.
          temp = std::max(temp, 0.0f);
          // Convert color space from linear to sRGB.
          temp = srgb_converter::from_linear(temp);
          // Convert float to unorm.
          destination_pixel[destination_channel_index] =
            unorm_converter<destination_t, destination_channel_t>::from_float(
              temp);
        }
        else
          BOOST_ASSERT(false);
      }
      else if constexpr (SourcePixel::data_type == pixel_data_type::ufloat)
      {
        if constexpr (DestinationPixel::data_type == pixel_data_type::unorm)
        {
          // Clamp values above 1.0f.
          auto temp = std::min(
            static_cast<float>(source_pixel[source_channel_index]), 1.0f);
          // Convert float to unorm.
          destination_pixel[destination_channel_index] =
            unorm_converter<destination_t, destination_channel_t>::from_float(
              temp);
        }
        else if constexpr (DestinationPixel::data_type ==
                           pixel_data_type::snorm)
        {
          // Clamp values above 1.0f.
          auto temp = std::min(
            static_cast<float>(source_pixel[source_channel_index]), 1.0f);
          // Convert float to snorm.
          destination_pixel[destination_channel_index] =
            snorm_converter<destination_t, destination_channel_t>::from_float(
              temp);
        }
        else if constexpr (DestinationPixel::data_type ==
                           pixel_data_type::ufloat)
        {
          // Simply copy value.
          destination_pixel[destination_channel_index] =
            static_cast<float>(source_pixel[source_channel_index]);
        }
        else if constexpr (DestinationPixel::data_type ==
                           pixel_data_type::sfloat)
        {
          // Simply copy value.
          destination_pixel[destination_channel_index] =
            static_cast<destination_t>(source_pixel[source_channel_index]);
        }
        else if constexpr (DestinationPixel::data_type == pixel_data_type::srgb)
        {
          // Clamp values above 1.0f.
          auto temp = std::min(
            static_cast<float>(source_pixel[source_channel_index]), 1.0f);
          // Convert color space from linear to sRGB.
          temp = srgb_converter::from_linear(temp);
          // Convert float to unorm.
          destination_pixel[destination_channel_index] =
            unorm_converter<destination_t, destination_channel_t>::from_float(
              temp);
        }
        else
          BOOST_ASSERT(false);
      }
      else if constexpr (SourcePixel::data_type == pixel_data_type::sfloat)
      {
        if constexpr (DestinationPixel::data_type == pixel_data_type::unorm)
        {
          // Clamp values between 0.0f and 1.0f.
          auto temp = std::clamp(
            static_cast<float>(source_pixel[source_channel_index]), 0.0f, 1.0f);
          // Convert float to unorm.
          destination_pixel[destination_channel_index] =
            unorm_converter<destination_t, destination_channel_t>::from_float(
              temp);
        }
        else if constexpr (DestinationPixel::data_type ==
                           pixel_data_type::snorm)
        {
          // Clamp values between -1.0f and 1.0f.
          auto temp =
            std::clamp(static_cast<float>(source_pixel[source_channel_index]),
                       -1.0f, 1.0f);
          // Convert float to snorm.
          destination_pixel[destination_channel_index] =
            snorm_converter<destination_t, destination_channel_t>::from_float(
              temp);
        }
        else if constexpr (DestinationPixel::data_type ==
                           pixel_data_type::ufloat)
        {
          // Clamp values below 0.0f.
          destination_pixel[destination_channel_index] = std::max(
            static_cast<float>(source_pixel[source_channel_index]), 0.0f);
        }
        else if constexpr (DestinationPixel::data_type ==
                           pixel_data_type::sfloat)
        {
          // Simply copy value.
          destination_pixel[destination_channel_index] =
            static_cast<destination_t>(source_pixel[source_channel_index]);
        }
        else if constexpr (DestinationPixel::data_type == pixel_data_type::srgb)
        {
          // Clamp values between 0.0f and 1.0f.
          auto temp = std::clamp(
            static_cast<float>(source_pixel[source_channel_index]), 0.0f, 1.0f);
          // Convert color space from linear to sRGB.
          temp = srgb_converter::from_linear(temp);
          // Convert float to unorm.
          destination_pixel[destination_channel_index] =
            unorm_converter<destination_t, destination_channel_t>::from_float(
              temp);
        }
        else
          BOOST_ASSERT(false);
      }
      else if constexpr (SourcePixel::data_type == pixel_data_type::srgb)
      {
        if constexpr (DestinationPixel::data_type == pixel_data_type::unorm)
        {
          // Convert unorm to float.
          auto temp = unorm_converter<source_t, source_channel_t>::to_float(
            source_pixel[source_channel_index]);
          // Convert color space from sRGB to linear.
          temp = srgb_converter::to_linear(temp);
          // Convert float to unorm.
          destination_pixel[destination_channel_index] =
            unorm_converter<destination_t, destination_channel_t>::from_float(
              temp);
        }
        else if constexpr (DestinationPixel::data_type ==
                           pixel_data_type::snorm)
        {
          // Convert unorm to float.
          auto temp = unorm_converter<source_t, source_channel_t>::to_float(
            source_pixel[source_channel_index]);
          // Convert color space from sRGB to linear.
          temp = srgb_converter::to_linear(temp);
          // Convert float to snorm.
          destination_pixel[destination_channel_index] =
            snorm_converter<destination_t, destination_channel_t>::from_float(
              temp);
        }
        else if constexpr (DestinationPixel::data_type ==
                             pixel_data_type::ufloat ||
                           DestinationPixel::data_type ==
                             pixel_data_type::sfloat)
        {
          // Convert unorm to float.
          auto temp = unorm_converter<source_t, source_channel_t>::to_float(
            source_pixel[source_channel_index]);
          // Convert color space from sRGB to linear.
          destination_pixel[destination_channel_index] =
            srgb_converter::to_linear(temp);
        }
        else if constexpr (DestinationPixel::data_type == pixel_data_type::srgb)
        {
          // Convert unorm to float.
          auto temp = unorm_converter<source_t, source_channel_t>::to_float(
            source_pixel[source_channel_index]);
          // Convert float to unorm.
          destination_pixel[destination_channel_index] =
            unorm_converter<destination_t, destination_channel_t>::from_float(
              temp);
        }
        else
          BOOST_ASSERT(false);
      }
      else
        BOOST_ASSERT(false);
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
                  const source_pixel_t& source) const
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
