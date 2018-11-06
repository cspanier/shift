#ifndef SHIFT_RC_RESOURCECOMPILER_HPP
#define SHIFT_RC_RESOURCECOMPILER_HPP

#include <memory>
#include <string_view>
#include <boost/filesystem/path.hpp>
#include <shift/core/singleton.hpp>
#include <shift/core/exception.hpp>

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
  void input_path(const fs::path& path);

  ///
  const fs::path& build_path() const;

  ///
  void build_path(const fs::path& path);

  ///
  const fs::path& output_path() const;

  ///
  void output_path(const fs::path& path);

  ///
  std::uint32_t verbose() const;

  ///
  void verbose(std::uint32_t level);

  ///
  const fs::path& image_magick() const;

  ///
  void image_magick(const fs::path& path);

  ///
  void load_rules(const std::string_view rules_filename);

  ///
  bool load_cache(const boost::filesystem::path& cache_filename);

  ///
  void save_cache(const boost::filesystem::path& cache_filename);

  ///
  void save_cache_graph(const boost::filesystem::path& cache_graph_filename);

  ///
  std::tuple<std::size_t /*succeeded_job_count*/,
             std::size_t /*failed_job_count*/>
  update();

  /// Drop all resources that are neither references by any other resource nor
  /// marked as named resources.
  void collect_garbage();

private:
  std::unique_ptr<resource_compiler_impl> _impl;
};
}

#endif
