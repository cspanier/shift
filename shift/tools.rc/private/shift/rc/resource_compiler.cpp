#include "shift/rc/resource_compiler.h"
#include "shift/rc/resource_compiler_impl.h"
#include "shift/rc/optimizer_mesh/filter.h"
#include "shift/task/async.h"
#include <shift/log/log.h>
#include <shift/core/stream_util.h>
#include <boost/iostreams/device/file.hpp>
#include <boost/filesystem.hpp>

namespace shift::rc
{
resource_compiler::resource_compiler()
: _impl(std::make_unique<resource_compiler_impl>())
{
}

resource_compiler::~resource_compiler() = default;

const fs::path& resource_compiler::input_path() const
{
  return _impl->input_path;
}

void resource_compiler::input_path(const fs::path& value)
{
  std::unique_lock lock(_impl->global_mutex);

  if (!fs::exists(value))
  {
    BOOST_THROW_EXCEPTION(rc_error()
                          << core::context_info("Input path does not exist"));
  }
  if (!fs::is_directory(value))
  {
    BOOST_THROW_EXCEPTION(
      rc_error() << core::context_info("Input path is not a directory"));
  }
  _impl->input_path = value;
}

const fs::path& resource_compiler::build_path() const
{
  return _impl->build_path;
}

void resource_compiler::build_path(const fs::path& value)
{
  std::unique_lock lock(_impl->global_mutex);

  if (!fs::exists(value))
  {
    BOOST_THROW_EXCEPTION(rc_error()
                          << core::context_info("Build path does not exist"));
  }
  if (!fs::is_directory(value))
  {
    BOOST_THROW_EXCEPTION(
      rc_error() << core::context_info("Build path is not a directory"));
  }
  _impl->build_path = value;
}

const fs::path& resource_compiler::output_path() const
{
  return _impl->output_path;
}

void resource_compiler::output_path(const fs::path& value)
{
  std::unique_lock lock(_impl->global_mutex);

  if (!fs::exists(value))
  {
    BOOST_THROW_EXCEPTION(rc_error()
                          << core::context_info("Output path does not exist"));
  }
  if (!fs::is_directory(value))
  {
    BOOST_THROW_EXCEPTION(
      rc_error() << core::context_info("Output path is not a directory"));
  }
  _impl->output_path = value;
}

std::uint32_t resource_compiler::verbose() const
{
  return _impl->verbose;
}

void resource_compiler::verbose(std::uint32_t level)
{
  _impl->verbose = level;
}

const fs::path& resource_compiler::image_magick() const
{
  return _impl->image_magick;
}

void resource_compiler::image_magick(const fs::path& value)
{
  std::unique_lock lock(_impl->global_mutex);
  _impl->image_magick = value;
}

void resource_compiler::load_rules(const fs::path& rules_filename)
{
  std::unique_lock lock(_impl->global_mutex);
  if (verbose() >= 1)
    log::info() << "Loading rule files...";
  /// ToDo: We certainly need to clear more data when rebuilding the rules list.
  _impl->rules.clear();

  // Scan for rules files.
  for (auto input_iterator = fs::recursive_directory_iterator(
         _impl->input_path, fs::symlink_option::no_recurse);
       input_iterator != fs::recursive_directory_iterator(); ++input_iterator)
  {
    const auto& rules_file_path = input_iterator->path();
    if (fs::is_regular_file(rules_file_path) &&
        rules_file_path.filename() == rules_filename)
    {
      if (verbose())
        log::info() << "Loading rule file " << rules_file_path;
      auto rule_path =
        fs::relative(rules_file_path, _impl->input_path).remove_filename();
      _impl->read_rules(rules_file_path, rule_path);
    }
  }
}

bool resource_compiler::load_cache(
  const boost::filesystem::path& cache_filename)
{
  std::unique_lock lock(_impl->global_mutex);

  if (verbose() >= 1)
    log::info() << "Loading cache file " << cache_filename << "...";

  return _impl->cache.load(cache_filename);
}

void resource_compiler::save_cache(
  const boost::filesystem::path& cache_filename)
{
  std::unique_lock lock(_impl->global_mutex);

  if (verbose() >= 1)
    log::info() << "Saving cache file " << cache_filename << "...";

  _impl->cache.save(cache_filename);
}

void resource_compiler::update()
{
  std::unique_lock lock(_impl->global_mutex);

  // Build a list of jobs from all files matching one of the rules.
  std::uint32_t current_pass = 0;
  for (auto input_iterator =
         fs::recursive_directory_iterator(_impl->input_path);
       input_iterator != fs::recursive_directory_iterator(); ++input_iterator)
  {
    const auto& file_path = input_iterator->path();
    if (fs::is_regular_file(file_path))
      _impl->match_file(_impl->add_file(file_path, current_pass), current_pass);
  }

  // Loop until there is no more work to do.
  /// ToDo: Add method to quit this loop early.
  for (;;)
  {
    std::vector<job_description*> jobs;
    {
      std::shared_lock read_lock(_impl->rules_mutex);
      do
      {
        current_pass = _impl->next_pass(current_pass, read_lock);
        if (current_pass == 0)
          break;
        jobs = _impl->query_jobs(current_pass, read_lock);
      } while (jobs.empty());
    }
    if (current_pass == 0)
      break;

    // Spawn an own parallel task for each job in the current pass.
    std::size_t modified_jobs_count = 0;
    for (auto* job : jobs)
    {
      if (job->flags.test(entity_flag::modified))
        ++modified_jobs_count;
    }
    log::info() << "Queuing " << modified_jobs_count
                << " modified jobs(s) in pass " << current_pass << " (skipping "
                << (jobs.size() - modified_jobs_count)
                << " unmodified job(s))...";
    std::vector<task::future<bool>> task_results;
    task_results.reserve(jobs.size());
    auto process_job = [&](rc::job_description* job) -> bool {
      /// ToDo: Change job parameter of reference type.
      if (!job)
        return false;

      BOOST_ASSERT(job->matching_rule);
      BOOST_ASSERT(job->matching_rule->action);
      if (!job->matching_rule)
        return false;
      if (!job->matching_rule->action)
        return false;

      bool result = false;
      // Only process the job if it is modified.
      if (job->flags.test(entity_flag::modified))
      {
        try
        {
          result = job->matching_rule->action->process(*_impl, *job);
          if (verbose() >= 1)
          {
            log::info line;
            bool first = true;
            line << "(";
            for (const auto& input : job->inputs)
            {
              if (first)
                first = false;
              else
                line << ", ";
              line << input->file->generic_string;
            }
            line << ") -> (";
            first = true;
            for (const auto& output : job->outputs)
            {
              if (first)
                first = false;
              else
                line << ", ";
              line << output->generic_string;
            }
            line << ")";
          }
        }
        catch (boost::exception& e)
        {
          {
            log::info line;
            bool first = true;
            line << "(";
            for (const auto& input : job->inputs)
            {
              if (first)
                first = false;
              else
                line << ", ";
              line << input->file->generic_string;
            }
            line << ") -> failed";
          }
          log::exception() << boost::diagnostic_information(e);
        }
      }
      else
        result = true;

      if (result)
      {
        // Push all output files into the pipeline.
        for (auto* output_file : job->outputs)
          _impl->match_file(output_file, job->matching_rule->pass);
      }
      else
      {
        job->flags.set(entity_flag::failed);
        log::error() << "A job using rule " << job->matching_rule->id
                     << " failed.";
      }
      return false;
    };

    // Spawn all jobs that don't support multithreading serially.
    // Use an empty dummy task to ease initialization of serial_result.
    auto serial_result = task::async([]() -> bool { return true; });
    for (auto* job : jobs)
    {
      if (job->matching_rule->action->support_multithreading())
        continue;

      serial_result = std::move(serial_result)
                        .then(
                          [&](task::future<bool> /* previous_result */,
                              rc::job_description* job) -> bool {
                            // Ignore value of previous_result and proceed with
                            // next job in any case.
                            return process_job(job);
                          },
                          job);
    }
    // Add the last serial_result to the list of futures to wait on.
    task_results.push_back(std::move(serial_result));

    // Spawn all jobs that do support multithreading in parallel.
    for (auto* job : jobs)
    {
      if (!job->matching_rule->action->support_multithreading())
        continue;

      task_results.emplace_back(task::async(process_job, job));
    }

    // Wait until all tasks completed.
    task::when_all(begin(task_results), end(task_results)).get();
  }
}

void resource_compiler::collect_garbage()
{
  std::unique_lock lock(_impl->global_mutex);

  // Remove all files which were read from cache but not used in this run.
  for (const auto& file : _impl->files)
  {
    if (file.second->flags & entity_flag::used)
      continue;
    log::info() << "Removing file " << file.first;
    /// ToDo: Actually remove files once we're sure that we won't delete
    /// anything important. Eventually add an additional security mechanism to
    /// avoid deleting files from private folder.
    /// Note that we must not simply use boost filesystem here but we have to
    /// use the repository's delete function.
  }

  // for (;;)
  //{
  //  // Copy all resource ids.
  //  auto unreferenced_resources = _impl->resource_ids;
  //  // Exclude all resource which are being referenced by some other
  //  resource.
  //  for (auto dependency : _impl->dependencies)
  //    unreferenced_resources.erase(dependency.second);
  //  // Exclude all resources that have a name.
  //  for (auto name_mapping : _impl->name_mappings)
  //    unreferenced_resources.erase(name_mapping.second);
  //  // Exit infinite loop once we don't have any garbage left.
  //  if (unreferenced_resources.empty())
  //    return;
  //  for (auto id : unreferenced_resources)
  //  {
  //    // Remove resource from global set.
  //    _impl->resource_ids.erase(id);
  //    // Erase resource storage.
  //    for (auto& mount_point : _impl->mount_points)
  //      mount_point->erase(id);
  //  }
  //}
}
}
