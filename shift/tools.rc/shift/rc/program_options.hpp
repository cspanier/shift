#ifndef SHIFT_RC_PROGRAMOPTIONS_HPP
#define SHIFT_RC_PROGRAMOPTIONS_HPP

#include <filesystem>

namespace shift
{
namespace rc
{
  struct program_options
  {
    static std::filesystem::path input_path;
    static std::filesystem::path build_path;
    static std::filesystem::path output_path;
    static std::string rules_filename;
    static std::string cache_filename;
    static std::uint32_t verbose;
    static std::string image_magick;
  };
}
}

#endif
