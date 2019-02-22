#ifndef TEST_SHIFT_RC_UTILITY_H
#define TEST_SHIFT_RC_UTILITY_H

#include <string>
#include <filesystem>

struct settings_t
{
  std::filesystem::path root_path;
  std::filesystem::path source_path;
  std::filesystem::path input_path;
  std::filesystem::path build_path;
  std::filesystem::path output_path;
  std::string cache_filename = ".rc-cache.json";
  std::string rules_filename = ".rc-rules.json";
};

std::filesystem::path working_path();

settings_t create_working_folders();

void run_rc(const settings_t& settings, std::size_t expect_succeeded,
            std::size_t expect_failed);

void remove_working_folders(const settings_t& settings);

/// Copies all files whose names match the regular expression from
/// source_folder to target_folder.
void copy_files(const std::filesystem::path& source_folder,
                const std::filesystem::path& target_folder,
                std::string regex_pattern);

void write_text_file(const std::filesystem::path& filename,
                     std::string_view content);

void write_png_image(const std::filesystem::path& filename, std::uint32_t width,
                     std::uint32_t height, std::uint32_t rgba);

#endif
