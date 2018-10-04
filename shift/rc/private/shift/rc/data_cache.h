#ifndef SHIFT_RC_DATA_CACHE_H
#define SHIFT_RC_DATA_CACHE_H

#include <boost/filesystem/path.hpp>
#include "shift/rc/types.h"

namespace shift::rc
{
class resource_compiler_impl;

///
struct job_hasher
{
  std::size_t operator()(
    const std::unique_ptr<shift::rc::job_description>& job) const
  {
    BOOST_ASSERT(job);
    if (!job)
      BOOST_THROW_EXCEPTION(core::logic_error());
    return std::hash<shift::rc::job_description>{}(*job);
  }
};

///
struct job_comparator
{
  bool operator()(const std::unique_ptr<shift::rc::job_description>& lhs,
                  const std::unique_ptr<shift::rc::job_description>& rhs) const
  {
    BOOST_ASSERT(lhs && rhs);
    if (!lhs || !rhs)
      BOOST_THROW_EXCEPTION(core::logic_error());
    return *lhs == *rhs;
  }
};

/// The cache data structure holds all information needed to decide whether a
/// certain job needs to be rerun or not.
class data_cache
{
public:
  /// Constructor.
  data_cache(resource_compiler_impl& impl);

  /// Destructor.
  ~data_cache();

  /// Loads cached data from a previously saved JSON file.
  /// @pre
  ///   All actions have been registered and all rules have been read from file.
  /// @remarks
  ///   Actions, rules, jobs and files are initially created in modified state.
  ///   Only those that are found in this cache are reset to unmodified. Note
  ///   that jobs and files are created dynamically during run-time and their
  ///   modification state get evaluated later on.
  bool load(const boost::filesystem::path& cache_filename);

  ///
  void save(const boost::filesystem::path& cache_filename);

  /// Looks up a file in the list of cached files.
  file_stats* get_file(const fs::path& file_path);

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
  bool is_modified(const file_stats& file) const;

private:
  resource_compiler_impl* _impl = nullptr;
  std::unordered_map<fs::path, std::unique_ptr<file_stats>> _files;
  std::unordered_set<std::unique_ptr<job_description>, job_hasher,
                     job_comparator>
    _jobs;
};
}

#endif
