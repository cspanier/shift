#ifndef SHIFT_RC_TIFF_IO_HPP
#define SHIFT_RC_TIFF_IO_HPP

#include <cstdint>
#include <vector>
#include <filesystem>
#include <shift/core/singleton.hpp>

namespace shift::rc::tiff
{
///
struct image
{
  std::uint16_t samples_per_pixel = 0;
  std::uint16_t bits_per_sample = 0;
  std::uint16_t samples_format = 0;
  std::uint16_t compression = 0;
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::uint32_t rows_per_strip = 0;

  std::vector<std::byte> pixel_data;
};

///
class io : public core::singleton<io, core::create::on_stack>
{
public:
  /// Constructor.
  io();

  ///
  bool load(const std::filesystem::path& filename, std::vector<image>& images);

  ///
  bool save(const std::filesystem::path& filename,
            const std::vector<image>& images);
};
}

#endif
