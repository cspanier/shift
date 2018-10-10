#ifndef SHIFT_RC_RESOURCECOMPILER_H
#define SHIFT_RC_RESOURCECOMPILER_H

#include <memory>
#include <string_view>
#include <boost/filesystem/path.hpp>
#include <shift/core/singleton.h>
#include <shift/core/exception.h>

namespace shift::rc
{
namespace fs = boost::filesystem;

struct rc_error : virtual core::runtime_error
{
};

class resource_compiler_impl;

///
class resource_compiler
: public core::singleton<resource_compiler, core::create::on_stack>
{
public:
  /// Constructor.
  resource_compiler();

  /// Destructor.
  ~resource_compiler();

  ///
  const fs::path& input_path() const;

  ///
  void input_path(const fs::path& value);

  ///
  const fs::path& build_path() const;

  ///
  void build_path(const fs::path& value);

  ///
  const fs::path& output_path() const;

  ///
  void output_path(const fs::path& value);

  ///
  std::uint32_t verbose() const;

  ///
  void verbose(std::uint32_t level);

  ///
  const fs::path& image_magick() const;

  ///
  void image_magick(const fs::path& value);

  ///
  void load_rules(const std::string_view rules_filename);

  ///
  bool load_cache(const boost::filesystem::path& cache_filename);

  ///
  void save_cache(const boost::filesystem::path& cache_filename);

  ///
  void update();

  /// Drop all resources that are neither references by any other resource nor
  /// marked as named resources.
  void collect_garbage();

private:
  std::unique_ptr<resource_compiler_impl> _impl;
};
}

#endif
