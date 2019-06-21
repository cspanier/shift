#ifndef SHIFT_RC_IMAGE_UTIL_PIXEL_H
#define SHIFT_RC_IMAGE_UTIL_PIXEL_H

#include <cstdint>
#include <shift/core/algorithm.hpp>
#include <shift/core/mpl.hpp>
#include <shift/math/half.hpp>

namespace shift::rc::image_util
{
using ::shift::math::half;

/// @tparam SizeInBits
///   Size in bits of this channel's values.
template <std::size_t SizeInBits, std::uint32_t BlockWidth = 1,
          std::uint32_t BlockHeight = 1>
struct pixel_channel_t
{
  static constexpr std::size_t size_in_bits = SizeInBits;
  static constexpr std::uint32_t block_width = BlockWidth;
  static constexpr std::uint32_t block_height = BlockHeight;
  static constexpr bool is_block_format = block_width > 1 || block_height > 1;
};

/// Type for a red pixel channel.
template <std::size_t SizeInBits>
struct red_t : public pixel_channel_t<SizeInBits>
{
};

/// Type for a green pixel channel.
template <std::size_t SizeInBits>
struct green_t : public pixel_channel_t<SizeInBits>
{
};

/// Type for a blue pixel channel.
template <std::size_t SizeInBits>
struct blue_t : public pixel_channel_t<SizeInBits>
{
};

/// Type for an alpha pixel channel.
template <std::size_t SizeInBits>
struct alpha_t : public pixel_channel_t<SizeInBits>
{
};

/// Type for a BC1 compressed channel.
struct bc1_rgb_t : public pixel_channel_t<64, 4, 4>
{
};

/// Type for a BC1 compressed channel.
struct bc1_rgba_t : public pixel_channel_t<64, 4, 4>
{
};

/// Type for a BC2 compressed channel.
struct bc2_t : public pixel_channel_t<128, 4, 4>
{
};

/// Type for a BC3 compressed channel.
struct bc3_t : public pixel_channel_t<128, 4, 4>
{
};

/// Type for a BC4 compressed channel.
struct bc4_t : public pixel_channel_t<64, 4, 4>
{
};

/// Type for a BC5 compressed channel.
struct bc5_t : public pixel_channel_t<128, 4, 4>
{
};

/// Type for a BC6H compressed channel.
struct bc6h_t : public pixel_channel_t<128, 4, 4>
{
};

/// Type for a BC7 compressed channel.
struct bc7_t : public pixel_channel_t<128, 4, 4>
{
};

/// Type for an unused pixel channel that will be skipped while reading.
template <std::size_t SizeInBits>
struct unused_t : public pixel_channel_t<SizeInBits>
{
};

namespace detail
{
  /// Utility template to count the number of pixel channels that are not tagged
  /// as unused and that are left of Channel in the list of channels.
  template <typename ChannelVector, typename Channel>
  struct channel_traits;

  /// End recursion when the vector of channels is empty.
  template <typename Channel>
  struct channel_traits<core::vector<>, Channel>
  {
    static constexpr std::size_t index = 0;
    static constexpr std::size_t offset = 0;
  };

  /// End recursion when we found the channel to search for.
  template <template <std::size_t> typename Channel, std::size_t N,
            std::size_t M, typename... NextChannels>
  struct channel_traits<core::vector<Channel<N>, NextChannels...>, Channel<M>>
  {
    static constexpr std::size_t index = 0;
    static constexpr std::size_t offset = 0;
  };

  /// Don't count unused channels.
  template <std::size_t SizeInBits, typename... NextChannels, typename Channel>
  struct channel_traits<core::vector<unused_t<SizeInBits>, NextChannels...>,
                        Channel>
  {
    static constexpr std::size_t index =
      channel_traits<core::vector<NextChannels...>, Channel>::index;
    static constexpr std::size_t offset =
      channel_traits<core::vector<NextChannels...>, Channel>::offset;
  };

  /// Count all other channels.
  template <typename FirstChannel, typename... NextChannels, typename Channel>
  struct channel_traits<core::vector<FirstChannel, NextChannels...>, Channel>
  {
    static constexpr std::size_t index =
      1 + channel_traits<core::vector<NextChannels...>, Channel>::index;
    static constexpr std::size_t offset =
      FirstChannel::size_in_bits +
      channel_traits<core::vector<NextChannels...>, Channel>::offset;
  };

  template <typename ChannelVector, typename Channel>
  constexpr std::size_t channel_index_v =
    channel_traits<ChannelVector, Channel>::index;

  template <typename ChannelVector, typename Channel>
  constexpr std::size_t channel_offset_v =
    channel_traits<ChannelVector, Channel>::offset;

  /// Searching for the index of a non-existing channel in a vector of channels
  /// returns the number of channels in that vector.
  template <typename ChannelVector>
  constexpr std::size_t channel_count_v =
    channel_traits<ChannelVector, void>::index;
}

/// The color space of the pixel values.
enum class pixel_data_type
{
  unorm,
  snorm,
  ufloat,
  sfloat,
  srgb,
  other
};

/// This pixel type is used both for pixel layout description and for storing
/// pixels in an array of unpacked pixel components, which makes it easy to
/// access individual pixel components that may otherwise not be byte aligned.
/// @tparam ComponentType
///   Storage type of channel values. Supported types are
///   std::(u)int(8|16|32)_t, half, and float for unpacked formats and
///   std::array<std::uint64_t, N> for packed formats.
/// @tparam DataType
/// @tparam ChannelVector
///   A core::vector<...> containing a list of pixel_channel_t<> types.
template <typename ComponentType, pixel_data_type DataType,
          typename ChannelVector>
struct pixel_definition_t;

template <typename ComponentType, pixel_data_type DataType,
          typename... Channels>
struct pixel_definition_t<ComponentType, DataType, core::vector<Channels...>>
{
  static_assert(std::is_same_v<ComponentType, std::int8_t> ||
                std::is_same_v<ComponentType, std::uint8_t> ||
                std::is_same_v<ComponentType, std::int16_t> ||
                std::is_same_v<ComponentType, std::uint16_t> ||
                std::is_same_v<ComponentType, std::int32_t> ||
                std::is_same_v<ComponentType, std::uint32_t> ||
                std::is_same_v<ComponentType, half> ||
                std::is_same_v<ComponentType, float> ||
                std::is_same_v<ComponentType, std::array<std::uint64_t, 1>> ||
                std::is_same_v<ComponentType, std::array<std::uint64_t, 2>>);

  using component_t = ComponentType;
  static constexpr pixel_data_type data_type = DataType;
  using channels_t = core::vector<Channels...>;
  static constexpr std::size_t channel_count = sizeof...(Channels);

  static constexpr bool is_packed =
    ((Channels::size_in_bits != sizeof(component_t)) || ...);

  static constexpr std::size_t size_in_bits = (Channels::size_in_bits + ...);
  static_assert(size_in_bits > 0);
  static constexpr std::size_t size_in_bytes = (size_in_bits + 7) / 8;
  static_assert(size_in_bytes * 8 == size_in_bits);

  static constexpr std::uint32_t block_width =
    core::max(Channels::block_width...);
  static constexpr std::uint32_t block_height =
    core::max(Channels::block_height...);
  static_assert(((Channels::block_width == block_width) && ...),
                "All pixel channels must have the same block width.");
  static_assert(((Channels::block_height == block_height) && ...),
                "All pixel channels must have the same block height.");

  static constexpr bool is_block_format = (Channels::is_block_format || ...);
  static_assert(((Channels::is_block_format == is_block_format) && ...),
                "You cannot mix block compressed with regular pixel channels.");

  ///
  constexpr component_t& operator[](std::size_t index)
  {
    BOOST_ASSERT(index < channel_count);
    return _channels[index];
  }

  ///
  constexpr const component_t& operator[](std::size_t index) const
  {
    BOOST_ASSERT(index < channel_count);
    return _channels[index];
  }

private:
  component_t _channels[channel_count];
};

namespace detail
{
  /// Read a single pixel channel from memory.
  template <typename Pixel>
  struct read_pixel_channel
  {
    using pixel_t = Pixel;

    template <std::size_t SizeInBits>
    void operator()(const unused_t<SizeInBits>*, const std::byte*, Pixel&) const
    {
      // Skip unused pixel components.
    }

    template <typename Channel>
    void operator()(const Channel*, const std::byte* source, Pixel& pixel) const
    {
      constexpr auto size_in_bits = Channel::size_in_bits;
      constexpr auto size_in_bytes = (size_in_bits + 7) / 8;
      constexpr auto offset_in_bits =
        channel_offset_v<typename pixel_t::channels_t, Channel>;
      constexpr auto offset_in_bytes = (offset_in_bits + 7) / 8;

      if constexpr ((size_in_bits % 8 == 0) && (offset_in_bits % 8 == 0) &&
                    (size_in_bytes == sizeof(typename pixel_t::component_t)))
      {
        constexpr auto channel_index =
          channel_index_v<typename pixel_t::channels_t, Channel>;
        static_assert(channel_index <
                      channel_count_v<typename pixel_t::channels_t>);
        std::memcpy(&pixel[channel_index], source + offset_in_bytes,
                    size_in_bytes);
      }
      else
      {
        /// ToDo: Copy unaligned pixel channels...
        BOOST_ASSERT(false);
      }
    }
  };

  /// Write a single pixel channel to memory.
  template <typename Pixel>
  struct write_pixel_channel
  {
    using pixel_t = Pixel;

    template <std::size_t SizeInBits>
    void operator()(const unused_t<SizeInBits>*, std::byte*, const Pixel&) const
    {
      // Skip unused pixel components.
    }

    template <typename Channel>
    void operator()(const Channel*, std::byte* destination,
                    const Pixel& pixel) const
    {
      constexpr auto size_in_bits = Channel::size_in_bits;
      constexpr auto size_in_bytes = (size_in_bits + 7) / 8;
      constexpr auto offset_in_bits =
        channel_offset_v<typename pixel_t::channels_t, Channel>;
      constexpr auto offset_in_bytes = (offset_in_bits + 7) / 8;

      if constexpr ((size_in_bits % 8 == 0) && (offset_in_bits % 8 == 0) &&
                    (size_in_bytes == sizeof(typename pixel_t::component_t)))
      {
        constexpr auto channel_index =
          channel_index_v<typename pixel_t::channels_t, Channel>;
        static_assert(channel_index <
                      channel_count_v<typename pixel_t::channels_t>);
        std::memcpy(destination + offset_in_bytes, &pixel[channel_index],
                    size_in_bytes);
      }
      else
      {
        /// ToDo: Copy unaligned pixel channels...
        BOOST_ASSERT(false);
      }
    }
  };
}

using pixel_r8_unorm = pixel_definition_t<std::uint8_t, pixel_data_type::unorm,
                                          core::vector<red_t<8>>>;

using pixel_r8_snorm = pixel_definition_t<std::int8_t, pixel_data_type::snorm,
                                          core::vector<red_t<8>>>;

using pixel_r8_srgb = pixel_definition_t<std::uint8_t, pixel_data_type::srgb,
                                         core::vector<red_t<8>>>;

using pixel_r8g8_unorm =
  pixel_definition_t<std::uint8_t, pixel_data_type::unorm,
                     core::vector<red_t<8>, green_t<8>>>;

using pixel_r8g8_snorm = pixel_definition_t<std::int8_t, pixel_data_type::snorm,
                                            core::vector<red_t<8>, green_t<8>>>;

using pixel_r8g8_srgb = pixel_definition_t<std::uint8_t, pixel_data_type::srgb,
                                           core::vector<red_t<8>, green_t<8>>>;

using pixel_r8g8b8_unorm =
  pixel_definition_t<std::uint8_t, pixel_data_type::unorm,
                     core::vector<red_t<8>, green_t<8>, blue_t<8>>>;

using pixel_r8g8b8_snorm =
  pixel_definition_t<std::int8_t, pixel_data_type::snorm,
                     core::vector<red_t<8>, green_t<8>, blue_t<8>>>;

using pixel_r8g8b8_srgb =
  pixel_definition_t<std::uint8_t, pixel_data_type::srgb,
                     core::vector<red_t<8>, green_t<8>, blue_t<8>>>;

using pixel_b8g8r8_unorm =
  pixel_definition_t<std::uint8_t, pixel_data_type::unorm,
                     core::vector<blue_t<8>, green_t<8>, red_t<8>>>;

using pixel_b8g8r8_snorm =
  pixel_definition_t<std::int8_t, pixel_data_type::snorm,
                     core::vector<blue_t<8>, green_t<8>, red_t<8>>>;

using pixel_b8g8r8_srgb =
  pixel_definition_t<std::uint8_t, pixel_data_type::srgb,
                     core::vector<blue_t<8>, green_t<8>, red_t<8>>>;

using pixel_r8g8b8a8_unorm =
  pixel_definition_t<std::uint8_t, pixel_data_type::unorm,
                     core::vector<red_t<8>, green_t<8>, blue_t<8>, alpha_t<8>>>;

using pixel_r8g8b8a8_snorm =
  pixel_definition_t<std::int8_t, pixel_data_type::snorm,
                     core::vector<red_t<8>, green_t<8>, blue_t<8>, alpha_t<8>>>;

using pixel_r8g8b8a8_srgb =
  pixel_definition_t<std::uint8_t, pixel_data_type::srgb,
                     core::vector<red_t<8>, green_t<8>, blue_t<8>, alpha_t<8>>>;

using pixel_b8g8r8a8_unorm =
  pixel_definition_t<std::uint8_t, pixel_data_type::unorm,
                     core::vector<blue_t<8>, green_t<8>, red_t<8>, alpha_t<8>>>;

using pixel_b8g8r8a8_snorm =
  pixel_definition_t<std::int8_t, pixel_data_type::snorm,
                     core::vector<blue_t<8>, green_t<8>, red_t<8>, alpha_t<8>>>;

using pixel_b8g8r8a8_srgb =
  pixel_definition_t<std::uint8_t, pixel_data_type::srgb,
                     core::vector<blue_t<8>, green_t<8>, red_t<8>, alpha_t<8>>>;

using pixel_a8r8g8b8_unorm =
  pixel_definition_t<std::uint8_t, pixel_data_type::unorm,
                     core::vector<alpha_t<8>, red_t<8>, green_t<8>, blue_t<8>>>;

using pixel_a8r8g8b8_snorm =
  pixel_definition_t<std::int8_t, pixel_data_type::snorm,
                     core::vector<alpha_t<8>, red_t<8>, green_t<8>, blue_t<8>>>;

using pixel_a8r8g8b8_srgb =
  pixel_definition_t<std::uint8_t, pixel_data_type::srgb,
                     core::vector<alpha_t<8>, red_t<8>, green_t<8>, blue_t<8>>>;

using pixel_a8b8g8r8_unorm =
  pixel_definition_t<std::uint8_t, pixel_data_type::unorm,
                     core::vector<alpha_t<8>, blue_t<8>, green_t<8>, red_t<8>>>;

using pixel_a8b8g8r8_snorm =
  pixel_definition_t<std::int8_t, pixel_data_type::snorm,
                     core::vector<alpha_t<8>, blue_t<8>, green_t<8>, red_t<8>>>;

using pixel_a8b8g8r8_srgb =
  pixel_definition_t<std::uint8_t, pixel_data_type::srgb,
                     core::vector<alpha_t<8>, blue_t<8>, green_t<8>, red_t<8>>>;

using pixel_r16_unorm =
  pixel_definition_t<std::uint16_t, pixel_data_type::unorm,
                     core::vector<red_t<16>>>;

using pixel_r16_snorm = pixel_definition_t<std::int16_t, pixel_data_type::snorm,
                                           core::vector<red_t<16>>>;

using pixel_r16_srgb = pixel_definition_t<std::uint16_t, pixel_data_type::srgb,
                                          core::vector<red_t<16>>>;

using pixel_r16_sfloat =
  pixel_definition_t<half, pixel_data_type::sfloat, core::vector<red_t<16>>>;

using pixel_r16g16_unorm =
  pixel_definition_t<std::uint16_t, pixel_data_type::unorm,
                     core::vector<red_t<16>, green_t<16>>>;

using pixel_r16g16_snorm =
  pixel_definition_t<std::int16_t, pixel_data_type::snorm,
                     core::vector<red_t<16>, green_t<16>>>;

using pixel_r16g16_srgb =
  pixel_definition_t<std::uint16_t, pixel_data_type::srgb,
                     core::vector<red_t<16>, green_t<16>>>;

using pixel_r16g16_sfloat =
  pixel_definition_t<half, pixel_data_type::sfloat,
                     core::vector<red_t<16>, green_t<16>>>;

using pixel_r16g16b16_unorm =
  pixel_definition_t<std::uint16_t, pixel_data_type::unorm,
                     core::vector<red_t<16>, green_t<16>, blue_t<16>>>;

using pixel_r16g16b16_snorm =
  pixel_definition_t<std::int16_t, pixel_data_type::snorm,
                     core::vector<red_t<16>, green_t<16>, blue_t<16>>>;

using pixel_r16g16b16_srgb =
  pixel_definition_t<std::uint16_t, pixel_data_type::srgb,
                     core::vector<red_t<16>, green_t<16>, blue_t<16>>>;

using pixel_r16g16b16_sfloat =
  pixel_definition_t<half, pixel_data_type::sfloat,
                     core::vector<red_t<16>, green_t<16>, blue_t<16>>>;

using pixel_b16g16r16_unorm =
  pixel_definition_t<std::uint16_t, pixel_data_type::unorm,
                     core::vector<blue_t<16>, green_t<16>, red_t<16>>>;

using pixel_b16g16r16_snorm =
  pixel_definition_t<std::int16_t, pixel_data_type::snorm,
                     core::vector<blue_t<16>, green_t<16>, red_t<16>>>;

using pixel_b16g16r16_srgb =
  pixel_definition_t<std::uint16_t, pixel_data_type::srgb,
                     core::vector<blue_t<16>, green_t<16>, red_t<16>>>;

using pixel_b16g16r16_sfloat =
  pixel_definition_t<half, pixel_data_type::sfloat,
                     core::vector<blue_t<16>, green_t<16>, red_t<16>>>;

using pixel_r16g16b16a16_unorm = pixel_definition_t<
  std::uint16_t, pixel_data_type::unorm,
  core::vector<red_t<16>, green_t<16>, blue_t<16>, alpha_t<16>>>;

using pixel_r16g16b16a16_snorm = pixel_definition_t<
  std::int16_t, pixel_data_type::snorm,
  core::vector<red_t<16>, green_t<16>, blue_t<16>, alpha_t<16>>>;

using pixel_r16g16b16a16_srgb = pixel_definition_t<
  std::uint16_t, pixel_data_type::srgb,
  core::vector<red_t<16>, green_t<16>, blue_t<16>, alpha_t<16>>>;

using pixel_r16g16b16a16_sfloat = pixel_definition_t<
  half, pixel_data_type::sfloat,
  core::vector<red_t<16>, green_t<16>, blue_t<16>, alpha_t<16>>>;

using pixel_b16g16r16a16_unorm = pixel_definition_t<
  std::uint16_t, pixel_data_type::unorm,
  core::vector<blue_t<16>, green_t<16>, red_t<16>, alpha_t<16>>>;

using pixel_b16g16r16a16_snorm = pixel_definition_t<
  std::int16_t, pixel_data_type::snorm,
  core::vector<blue_t<16>, green_t<16>, red_t<16>, alpha_t<16>>>;

using pixel_b16g16r16a16_srgb = pixel_definition_t<
  std::uint16_t, pixel_data_type::srgb,
  core::vector<blue_t<16>, green_t<16>, red_t<16>, alpha_t<16>>>;

using pixel_b16g16r16a16_sfloat = pixel_definition_t<
  half, pixel_data_type::sfloat,
  core::vector<blue_t<16>, green_t<16>, red_t<16>, alpha_t<16>>>;

using pixel_a16r16g16b16_unorm = pixel_definition_t<
  std::uint16_t, pixel_data_type::unorm,
  core::vector<alpha_t<16>, red_t<16>, green_t<16>, blue_t<16>>>;

using pixel_a16r16g16b16_snorm = pixel_definition_t<
  std::int16_t, pixel_data_type::snorm,
  core::vector<alpha_t<16>, red_t<16>, green_t<16>, blue_t<16>>>;

using pixel_a16r16g16b16_srgb = pixel_definition_t<
  std::uint16_t, pixel_data_type::srgb,
  core::vector<alpha_t<16>, red_t<16>, green_t<16>, blue_t<16>>>;

using pixel_a16r16g16b16_sfloat = pixel_definition_t<
  half, pixel_data_type::sfloat,
  core::vector<alpha_t<16>, red_t<16>, green_t<16>, blue_t<16>>>;

using pixel_a16b16g16r16_unorm = pixel_definition_t<
  std::uint16_t, pixel_data_type::unorm,
  core::vector<alpha_t<16>, blue_t<16>, green_t<16>, red_t<16>>>;

using pixel_a16b16g16r16_snorm = pixel_definition_t<
  std::int16_t, pixel_data_type::snorm,
  core::vector<alpha_t<16>, blue_t<16>, green_t<16>, red_t<16>>>;

using pixel_a16b16g16r16_srgb = pixel_definition_t<
  std::uint16_t, pixel_data_type::srgb,
  core::vector<alpha_t<16>, blue_t<16>, green_t<16>, red_t<16>>>;

using pixel_a16b16g16r16_sfloat = pixel_definition_t<
  half, pixel_data_type::sfloat,
  core::vector<alpha_t<16>, blue_t<16>, green_t<16>, red_t<16>>>;

using pixel_r32_unorm =
  pixel_definition_t<std::uint32_t, pixel_data_type::unorm,
                     core::vector<red_t<32>>>;

using pixel_r32_snorm = pixel_definition_t<std::int32_t, pixel_data_type::snorm,
                                           core::vector<red_t<32>>>;

using pixel_r32_srgb = pixel_definition_t<std::uint32_t, pixel_data_type::srgb,
                                          core::vector<red_t<32>>>;

using pixel_r32_sfloat =
  pixel_definition_t<float, pixel_data_type::sfloat, core::vector<red_t<32>>>;

using pixel_r32g32_unorm =
  pixel_definition_t<std::uint32_t, pixel_data_type::unorm,
                     core::vector<red_t<32>, green_t<32>>>;

using pixel_r32g32_snorm =
  pixel_definition_t<std::int32_t, pixel_data_type::snorm,
                     core::vector<red_t<32>, green_t<32>>>;

using pixel_r32g32_srgb =
  pixel_definition_t<std::uint32_t, pixel_data_type::srgb,
                     core::vector<red_t<32>, green_t<32>>>;

using pixel_r32g32_sfloat =
  pixel_definition_t<float, pixel_data_type::sfloat,
                     core::vector<red_t<32>, green_t<32>>>;

using pixel_r32g32b32_unorm =
  pixel_definition_t<std::uint32_t, pixel_data_type::unorm,
                     core::vector<red_t<32>, green_t<32>, blue_t<32>>>;

using pixel_r32g32b32_snorm =
  pixel_definition_t<std::int32_t, pixel_data_type::snorm,
                     core::vector<red_t<32>, green_t<32>, blue_t<32>>>;

using pixel_r32g32b32_srgb =
  pixel_definition_t<std::uint32_t, pixel_data_type::srgb,
                     core::vector<red_t<32>, green_t<32>, blue_t<32>>>;

using pixel_r32g32b32_sfloat =
  pixel_definition_t<float, pixel_data_type::sfloat,
                     core::vector<red_t<32>, green_t<32>, blue_t<32>>>;

using pixel_b32g32r32_unorm =
  pixel_definition_t<std::uint32_t, pixel_data_type::unorm,
                     core::vector<blue_t<32>, green_t<32>, red_t<32>>>;

using pixel_b32g32r32_snorm =
  pixel_definition_t<std::int32_t, pixel_data_type::snorm,
                     core::vector<blue_t<32>, green_t<32>, red_t<32>>>;

using pixel_b32g32r32_srgb =
  pixel_definition_t<std::uint32_t, pixel_data_type::srgb,
                     core::vector<blue_t<32>, green_t<32>, red_t<32>>>;

using pixel_b32g32r32_sfloat =
  pixel_definition_t<float, pixel_data_type::sfloat,
                     core::vector<blue_t<32>, green_t<32>, red_t<32>>>;

using pixel_r32g32b32a32_unorm = pixel_definition_t<
  std::uint32_t, pixel_data_type::unorm,
  core::vector<red_t<32>, green_t<32>, blue_t<32>, alpha_t<32>>>;

using pixel_r32g32b32a32_snorm = pixel_definition_t<
  std::int32_t, pixel_data_type::snorm,
  core::vector<red_t<32>, green_t<32>, blue_t<32>, alpha_t<32>>>;

using pixel_r32g32b32a32_srgb = pixel_definition_t<
  std::uint32_t, pixel_data_type::srgb,
  core::vector<red_t<32>, green_t<32>, blue_t<32>, alpha_t<32>>>;

using pixel_r32g32b32a32_sfloat = pixel_definition_t<
  float, pixel_data_type::sfloat,
  core::vector<red_t<32>, green_t<32>, blue_t<32>, alpha_t<32>>>;

using pixel_b32g32r32a32_unorm = pixel_definition_t<
  std::uint32_t, pixel_data_type::unorm,
  core::vector<blue_t<32>, green_t<32>, red_t<32>, alpha_t<32>>>;

using pixel_b32g32r32a32_srgb = pixel_definition_t<
  std::uint32_t, pixel_data_type::srgb,
  core::vector<blue_t<32>, green_t<32>, red_t<32>, alpha_t<32>>>;

using pixel_b32g32r32a32_snorm = pixel_definition_t<
  std::int32_t, pixel_data_type::snorm,
  core::vector<blue_t<32>, green_t<32>, red_t<32>, alpha_t<32>>>;

using pixel_b32g32r32a32_sfloat = pixel_definition_t<
  float, pixel_data_type::sfloat,
  core::vector<blue_t<32>, green_t<32>, red_t<32>, alpha_t<32>>>;

using pixel_a32r32g32b32_unorm = pixel_definition_t<
  std::uint32_t, pixel_data_type::unorm,
  core::vector<alpha_t<32>, red_t<32>, green_t<32>, blue_t<32>>>;

using pixel_a32r32g32b32_snorm = pixel_definition_t<
  std::int32_t, pixel_data_type::snorm,
  core::vector<alpha_t<32>, red_t<32>, green_t<32>, blue_t<32>>>;

using pixel_a32r32g32b32_srgb = pixel_definition_t<
  std::uint32_t, pixel_data_type::srgb,
  core::vector<alpha_t<32>, red_t<32>, green_t<32>, blue_t<32>>>;

using pixel_a32r32g32b32_sfloat = pixel_definition_t<
  float, pixel_data_type::sfloat,
  core::vector<alpha_t<32>, red_t<32>, green_t<32>, blue_t<32>>>;

using pixel_a32b32g32r32_unorm = pixel_definition_t<
  std::uint32_t, pixel_data_type::unorm,
  core::vector<alpha_t<32>, blue_t<32>, green_t<32>, red_t<32>>>;

using pixel_a32b32g32r32_snorm = pixel_definition_t<
  std::int32_t, pixel_data_type::snorm,
  core::vector<alpha_t<32>, blue_t<32>, green_t<32>, red_t<32>>>;

using pixel_a32b32g32r32_srgb = pixel_definition_t<
  std::uint32_t, pixel_data_type::srgb,
  core::vector<alpha_t<32>, blue_t<32>, green_t<32>, red_t<32>>>;

using pixel_a32b32g32r32_sfloat = pixel_definition_t<
  float, pixel_data_type::sfloat,
  core::vector<alpha_t<32>, blue_t<32>, green_t<32>, red_t<32>>>;

using pixel_bc1_rgb_unorm =
  pixel_definition_t<std::array<std::uint64_t, 1>, pixel_data_type::unorm,
                     core::vector<bc1_rgb_t>>;
using pixel_bc1_rgb_srgb =
  pixel_definition_t<std::array<std::uint64_t, 1>, pixel_data_type::srgb,
                     core::vector<bc1_rgb_t>>;
using pixel_bc1_rgba_unorm =
  pixel_definition_t<std::array<std::uint64_t, 1>, pixel_data_type::unorm,
                     core::vector<bc1_rgba_t>>;
using pixel_bc1_rgba_srgb =
  pixel_definition_t<std::array<std::uint64_t, 1>, pixel_data_type::srgb,
                     core::vector<bc1_rgba_t>>;
using pixel_bc2_unorm =
  pixel_definition_t<std::array<std::uint64_t, 2>, pixel_data_type::unorm,
                     core::vector<bc2_t>>;
using pixel_bc2_srgb =
  pixel_definition_t<std::array<std::uint64_t, 2>, pixel_data_type::srgb,
                     core::vector<bc2_t>>;
using pixel_bc3_unorm =
  pixel_definition_t<std::array<std::uint64_t, 2>, pixel_data_type::unorm,
                     core::vector<bc3_t>>;
using pixel_bc3_srgb =
  pixel_definition_t<std::array<std::uint64_t, 2>, pixel_data_type::srgb,
                     core::vector<bc3_t>>;
using pixel_bc4_unorm =
  pixel_definition_t<std::array<std::uint64_t, 1>, pixel_data_type::unorm,
                     core::vector<bc4_t>>;
using pixel_bc4_snorm =
  pixel_definition_t<std::array<std::uint64_t, 1>, pixel_data_type::snorm,
                     core::vector<bc4_t>>;
using pixel_bc5_unorm =
  pixel_definition_t<std::array<std::uint64_t, 2>, pixel_data_type::unorm,
                     core::vector<bc5_t>>;
using pixel_bc5_snorm =
  pixel_definition_t<std::array<std::uint64_t, 2>, pixel_data_type::snorm,
                     core::vector<bc5_t>>;
using pixel_bc6h_ufloat =
  pixel_definition_t<std::array<std::uint64_t, 2>, pixel_data_type::ufloat,
                     core::vector<bc6h_t>>;
using pixel_bc6h_sfloat =
  pixel_definition_t<std::array<std::uint64_t, 2>, pixel_data_type::sfloat,
                     core::vector<bc6h_t>>;
using pixel_bc7_unorm =
  pixel_definition_t<std::array<std::uint64_t, 2>, pixel_data_type::unorm,
                     core::vector<bc7_t>>;
using pixel_bc7_srgb =
  pixel_definition_t<std::array<std::uint64_t, 2>, pixel_data_type::srgb,
                     core::vector<bc7_t>>;

#if 0
using pixel_formats =
  // A reduced set of pixel formats for debugging purposes.
  core::set<pixel_r8g8b8_unorm, pixel_r8g8b8a8_unorm, pixel_r16g16b16a16_unorm,
            pixel_bc1_rgb_unorm, pixel_bc1_rgb_srgb, pixel_bc1_rgba_unorm,
            pixel_bc3_unorm>;
#else
using pixel_formats = core::set<
  pixel_r8_unorm, pixel_r8_snorm, pixel_r8_srgb,

  pixel_r8g8_unorm, pixel_r8g8_snorm, pixel_r8g8_srgb,

  pixel_r8g8b8_unorm, pixel_r8g8b8_snorm, pixel_r8g8b8_srgb, pixel_b8g8r8_unorm,
  pixel_b8g8r8_snorm, pixel_b8g8r8_srgb,

  pixel_r8g8b8a8_unorm, pixel_r8g8b8a8_snorm, pixel_r8g8b8a8_srgb,
  pixel_b8g8r8a8_unorm, pixel_b8g8r8a8_snorm, pixel_b8g8r8a8_srgb,
  pixel_a8r8g8b8_unorm, pixel_a8r8g8b8_snorm, pixel_a8r8g8b8_srgb,
  pixel_a8b8g8r8_unorm, pixel_a8b8g8r8_snorm, pixel_a8b8g8r8_srgb,

  pixel_r16_unorm, pixel_r16_snorm, pixel_r16_srgb, pixel_r16_sfloat,

  pixel_r16g16_unorm, pixel_r16g16_snorm, pixel_r16g16_srgb,
  pixel_r16g16_sfloat,

  pixel_r16g16b16_unorm, pixel_r16g16b16_snorm, pixel_r16g16b16_srgb,
  pixel_r16g16b16_sfloat, pixel_b16g16r16_unorm, pixel_b16g16r16_snorm,
  pixel_b16g16r16_srgb, pixel_b16g16r16_sfloat,

  pixel_r16g16b16a16_unorm, pixel_r16g16b16a16_snorm, pixel_r16g16b16a16_srgb,
  pixel_r16g16b16a16_sfloat, pixel_b16g16r16a16_unorm, pixel_b16g16r16a16_snorm,
  pixel_b16g16r16a16_srgb, pixel_b16g16r16a16_sfloat, pixel_a16r16g16b16_unorm,
  pixel_a16r16g16b16_snorm, pixel_a16r16g16b16_srgb, pixel_a16r16g16b16_sfloat,
  pixel_a16b16g16r16_unorm, pixel_a16b16g16r16_snorm, pixel_a16b16g16r16_srgb,
  pixel_a16b16g16r16_sfloat,

  pixel_r32_unorm, pixel_r32_snorm, pixel_r32_srgb, pixel_r32_sfloat,

  pixel_r32g32_unorm, pixel_r32g32_snorm, pixel_r32g32_srgb,
  pixel_r32g32_sfloat,

  pixel_r32g32b32_unorm, pixel_r32g32b32_snorm, pixel_r32g32b32_srgb,
  pixel_r32g32b32_sfloat, pixel_b32g32r32_unorm, pixel_b32g32r32_snorm,
  pixel_b32g32r32_srgb, pixel_b32g32r32_sfloat,

  pixel_r32g32b32a32_unorm, pixel_r32g32b32a32_snorm, pixel_r32g32b32a32_srgb,
  pixel_r32g32b32a32_sfloat, pixel_b32g32r32a32_unorm, pixel_b32g32r32a32_snorm,
  pixel_b32g32r32a32_srgb, pixel_b32g32r32a32_sfloat, pixel_a32r32g32b32_unorm,
  pixel_a32r32g32b32_snorm, pixel_a32r32g32b32_srgb, pixel_a32r32g32b32_sfloat,
  pixel_a32b32g32r32_unorm, pixel_a32b32g32r32_snorm, pixel_a32b32g32r32_srgb,
  pixel_a32b32g32r32_sfloat, pixel_bc1_rgb_unorm, pixel_bc1_rgb_srgb,
  pixel_bc1_rgba_unorm, pixel_bc1_rgba_srgb, pixel_bc2_unorm, pixel_bc2_srgb,
  pixel_bc3_unorm, pixel_bc3_srgb, pixel_bc4_unorm, pixel_bc4_snorm,
  pixel_bc5_unorm, pixel_bc5_snorm, pixel_bc6h_ufloat, pixel_bc6h_sfloat,
  pixel_bc7_unorm, pixel_bc7_srgb>;
#endif

/// A type mapping from block compressed pixel format components (64 or 128 bit)
/// to uncompressed pixel component types (std::uint8_t or float).
template <pixel_data_type DataType, typename ChannelVector>
struct uncompressed_pixel_component;

template <>
struct uncompressed_pixel_component<pixel_data_type::unorm,
                                    core::vector<bc1_rgb_t>>
{
  using type = std::uint8_t;
};

template <>
struct uncompressed_pixel_component<pixel_data_type::srgb,
                                    core::vector<bc1_rgb_t>>
{
  using type = std::uint8_t;
};

template <>
struct uncompressed_pixel_component<pixel_data_type::unorm,
                                    core::vector<bc1_rgba_t>>
{
  using type = std::uint8_t;
};

template <>
struct uncompressed_pixel_component<pixel_data_type::srgb,
                                    core::vector<bc1_rgba_t>>
{
  using type = std::uint8_t;
};

template <>
struct uncompressed_pixel_component<pixel_data_type::unorm, core::vector<bc2_t>>
{
  using type = std::uint8_t;
};

template <>
struct uncompressed_pixel_component<pixel_data_type::srgb, core::vector<bc2_t>>
{
  using type = std::uint8_t;
};

template <>
struct uncompressed_pixel_component<pixel_data_type::unorm, core::vector<bc3_t>>
{
  using type = std::uint8_t;
};

template <>
struct uncompressed_pixel_component<pixel_data_type::srgb, core::vector<bc3_t>>
{
  using type = std::uint8_t;
};

template <>
struct uncompressed_pixel_component<pixel_data_type::unorm, core::vector<bc4_t>>
{
  using type = std::uint8_t;
};

template <>
struct uncompressed_pixel_component<pixel_data_type::snorm, core::vector<bc4_t>>
{
  using type = std::int8_t;
};

template <>
struct uncompressed_pixel_component<pixel_data_type::unorm, core::vector<bc5_t>>
{
  using type = std::uint8_t;
};

template <>
struct uncompressed_pixel_component<pixel_data_type::snorm, core::vector<bc5_t>>
{
  using type = std::int8_t;
};

template <>
struct uncompressed_pixel_component<pixel_data_type::ufloat,
                                    core::vector<bc6h_t>>
{
  using type = float;
};

template <>
struct uncompressed_pixel_component<pixel_data_type::sfloat,
                                    core::vector<bc6h_t>>
{
  using type = float;
};

template <>
struct uncompressed_pixel_component<pixel_data_type::unorm, core::vector<bc7_t>>
{
  using type = std::uint8_t;
};

template <>
struct uncompressed_pixel_component<pixel_data_type::srgb, core::vector<bc7_t>>
{
  using type = std::uint8_t;
};

template <pixel_data_type DataType, typename ChannelVector>
using uncompressed_pixel_component_t =
  typename uncompressed_pixel_component<DataType, ChannelVector>::type;

///
template <typename ChannelVector>
struct uncompressed_pixel_channels;

template <>
struct uncompressed_pixel_channels<core::vector<bc1_rgb_t>>
{
  using type = core::vector<red_t<8>, green_t<8>, blue_t<8>, alpha_t<8>>;
};

template <>
struct uncompressed_pixel_channels<core::vector<bc1_rgba_t>>
{
  using type = core::vector<red_t<8>, green_t<8>, blue_t<8>, alpha_t<8>>;
};

template <>
struct uncompressed_pixel_channels<core::vector<bc2_t>>
{
  using type = core::vector<red_t<8>, green_t<8>, blue_t<8>, alpha_t<8>>;
};

template <>
struct uncompressed_pixel_channels<core::vector<bc3_t>>
{
  using type = core::vector<red_t<8>, green_t<8>, blue_t<8>, alpha_t<8>>;
};

template <>
struct uncompressed_pixel_channels<core::vector<bc4_t>>
{
  using type = core::vector<red_t<8>, green_t<8>, blue_t<8>, alpha_t<8>>;
};

template <>
struct uncompressed_pixel_channels<core::vector<bc5_t>>
{
  using type = core::vector<red_t<8>, green_t<8>, blue_t<8>, alpha_t<8>>;
};

template <>
struct uncompressed_pixel_channels<core::vector<bc6h_t>>
{
  using type = core::vector<red_t<16>, green_t<16>, blue_t<16>, alpha_t<16>>;
};

template <>
struct uncompressed_pixel_channels<core::vector<bc7_t>>
{
  using type = core::vector<red_t<8>, green_t<8>, blue_t<8>, alpha_t<8>>;
};

template <typename ChannelVector>
using uncompressed_pixel_channels_t =
  typename uncompressed_pixel_channels<ChannelVector>::type;
}

#endif
