#ifndef SHIFT_RC_RESOURCE_COMPILER_IMPL_HPP
#define SHIFT_RC_RESOURCE_COMPILER_IMPL_HPP

#include <vector>
#include <queue>
#include <unordered_map>
#include <shared_mutex>
#include <shift/resource_db/resource.hpp>
#include "shift/rc/types.hpp"
#include "shift/rc/data_cache.hpp"
#include "shift/rc/resource_compiler.hpp"

namespace shift::rc
{
///
class resource_compiler_impl
{
public:
  /// Default constructor.
  resource_compiler_impl();

  /// Destructor.
  ~resource_compiler_impl();

  ///
  void read_rules(const fs::path& rules_file_path, const fs::path& rule_path);

  /// Saves the resource and associates the output file with the job.
  file_description* save(const resource_db::resource_base& resource,
                         const fs::path& target_name, job_description& job);

  /// Associates the output file with the job.
  file_description* push(const fs::path& target_name, job_description& job);

  /// Tries to resolve an alias. If is not found in the internal dictionary
  /// source is returned instead.
  file_description* alias(file_description* source, std::uint32_t current_pass);

  /// Adds a file to the list of used files.
  /// @param pass
  ///   Define in which pass the file was generated. Use zero to express that it
  ///   is a source file. Files only match against rules of subsequent passes to
  ///   avoid dependency loops.
  /// @pre
  ///   The file identified by file_path exists and is a regular file.
  file_description* add_file(const fs::path& file_path, std::uint32_t pass);

  /// Looks up a file in the list of previously added files.
  file_description* get_file(const fs::path& file_path);

  /// Try to match a file against the currently loaded rule set, eventually
  /// adding a new match to a rule's matches vector.
  void match_file(file_description& file, std::uint32_t current_pass);

  /// Walk through the currently loaded rule set and finds the next pass that
  /// contains matches which eventually transform to jobs.
  /// @return
  ///   Either the next valid pass always larger than previous_pass, or zero
  ///   if there aren't any more matches in higher passes available.
  std::uint32_t next_pass(
    std::uint32_t previous_pass,
    std::shared_lock<std::shared_mutex>& /* rules_read_lock */);

  /// Walk through the currently loaded rule set and generates a list of jobs
  /// for the pass selected.
  std::vector<std::unique_ptr<job_description>> query_jobs(
    std::uint32_t pass, std::shared_lock<std::shared_mutex>& /* read_lock */);

  fs::path input_path;
  fs::path build_path;
  fs::path output_path;
  std::string rules_filename;
  std::uint32_t verbose = 0;
  fs::path image_magick;

  /// This mutex is used to prevent any external API calls while the resource
  /// compiler is working.
  std::mutex global_mutex;
  std::shared_mutex rules_mutex;
  std::vector<rule_description*> rules;
  data_cache cache;

  std::shared_mutex files_mutex;
  std::unordered_map<fs::path, std::unique_ptr<file_description>> files;
};
}

#endif
