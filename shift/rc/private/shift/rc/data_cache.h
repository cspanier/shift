#ifndef SHIFT_RC_DATA_CACHE_H
#define SHIFT_RC_DATA_CACHE_H

#include <boost/filesystem/path.hpp>
#include "shift/rc/types.h"

namespace shift::rc
{
class resource_compiler_impl;

/// The cache data structure holds all information needed to decide whether a
/// certain job needs to be rerun or not.
class data_cache
{
public:
  /// Constructor.
  data_cache(resource_compiler_impl& impl);

  /// Destructor.
  ~data_cache();

  /// Registers an action and marks it as modified.
  /// @pre
  ///   load() has not been called, yet.
  void register_action(std::string name, action_version version,
                       action_base& impl);

  /// Loads cached data from a previously saved JSON file.
  /// @pre
  ///   All built-in actions must be registered before loading the cache file.
  /// @post
  ///   All actions that were found in the cache file and that had the same
  ///   version are marked as unmodified.
  /// @remarks
  ///   Rules, jobs and files are initially created in modified state.
  ///   Only those that are found in this cache are reset to unmodified. Note
  ///   that jobs and files are created dynamically during run-time and their
  ///   modification state get evaluated later on.
  bool load(const boost::filesystem::path& cache_filename);

  /// Saves all cached data to a JSON file.
  void save(const boost::filesystem::path& cache_filename) const;

  /// Saves a GraphViz document of the file cache.
  void save_graph(const fs::path& graph_filename) const;

  ///
  action_description* find_action(std::string_view name) const;

  /// @see get_file(std::string_view).
  file_description* get_file(const fs::path& file_path);

  /// Looks up a file in the list of cached files.
  /// @return
  ///   Either a pointer to a valid file_descriptor object, or nullptr if no
  ///   file with the queried name was found.
  file_description* get_file(std::string_view file_path);

  /// Looks up a job in the list of cached jobs that is equivalent to the passed
  /// one.
  /// @remarks
  ///   Cached jobs contain the list of output files, which new jobs only know
  ///   after processing. This method is used on unmodified jobs to skip
  ///   processing and directly proceeding with any output.
  const job_description* get_job(const job_description& job) const;

  /// Checks whether a job is modified compared to the ones cached.
  bool is_modified(const job_description& job) const;

  /// Checks whether a file is modified compared to the ones cached.
  bool is_modified(const file_description& file) const;

private:
  resource_compiler_impl* _impl = nullptr;
  std::unordered_map<std::string_view, std::unique_ptr<action_description>>
    _actions;
  std::unordered_map<std::string_view, std::unique_ptr<rule_description>>
    _rules;
  std::unordered_map<std::string_view, std::unique_ptr<file_description>>
    _files;
  std::unordered_map<std::size_t, std::unique_ptr<job_description>> _jobs;
};
}

#endif
