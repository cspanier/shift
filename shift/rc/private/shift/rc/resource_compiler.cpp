#include "shift/rc/resource_compiler.hpp"
#include "shift/rc/resource_compiler_impl.hpp"
#include "shift/rc/optimizer_mesh/filter.hpp"
#include "shift/task/async.hpp"
#include <shift/log/log.hpp>
#include <shift/core/stream_util.hpp>
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

void resource_compiler::input_path(const fs::path& path)
{
  std::unique_lock lock(_impl->global_mutex);

  if (!fs::exists(path))
  {
    BOOST_THROW_EXCEPTION(rc_error()
                          << core::context_info("Input path does not exist"));
  }
  if (!fs::is_directory(path))
  {
    BOOST_THROW_EXCEPTION(
      rc_error() << core::context_info("Input path is not a directory"));
  }
  _impl->input_path =
    fs::relative(fs::canonical(fs::absolute(path)), fs::current_path());
}

const fs::path& resource_compiler::build_path() const
{
  return _impl->build_path;
}

void resource_compiler::build_path(const fs::path& path)
{
  std::unique_lock lock(_impl->global_mutex);

  if (!fs::exists(path))
  {
    BOOST_THROW_EXCEPTION(rc_error()
                          << core::context_info("Build path does not exist"));
  }
  if (!fs::is_directory(path))
  {
    BOOST_THROW_EXCEPTION(
      rc_error() << core::context_info("Build path is not a directory"));
  }
  _impl->build_path =
    fs::relative(fs::canonical(fs::absolute(path)), fs::current_path());
}

const fs::path& resource_compiler::output_path() const
{
  return _impl->output_path;
}

void resource_compiler::output_path(const fs::path& path)
{
  std::unique_lock lock(_impl->global_mutex);

  if (!fs::exists(path))
  {
    BOOST_THROW_EXCEPTION(rc_error()
                          << core::context_info("Output path does not exist"));
  }
  if (!fs::is_directory(path))
  {
    BOOST_THROW_EXCEPTION(
      rc_error() << core::context_info("Output path is not a directory"));
  }
  _impl->output_path =
    fs::relative(fs::canonical(fs::absolute(path)), fs::current_path());
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

void resource_compiler::image_magick(const fs::path& path)
{
  std::unique_lock lock(_impl->global_mutex);
  _impl->image_magick = path;
}

void resource_compiler::load_rules(const std::string_view rules_filename)
{
  std::unique_lock lock(_impl->global_mutex);
  if (verbose() >= 1)
    log::info() << "Loading rule files...";
  /// ToDo: We certainly need to clear more data when rebuilding the rules list.
  _impl->rules.clear();

  _impl->rules_filename = rules_filename;

  // Scan for rules files.
  for (auto input_iterator = fs::recursive_directory_iterator(
         _impl->input_path, fs::symlink_option::no_recurse);
       input_iterator != fs::recursive_directory_iterator(); ++input_iterator)
  {
    const auto& rules_file_path = input_iterator->path();
    if (fs::is_regular_file(rules_file_path) &&
        rules_file_path.filename() == _impl->rules_filename)
    {
      if (verbose() != 0u)
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

  if (verbose() >= 1)
    log::info() << "Saving cache graph " << cache_filename << "...";
  _impl->cache.save_graph(fs::path{cache_filename}.replace_extension(".dot"));
}

void resource_compiler::save_cache_graph(
  const boost::filesystem::path& cache_graph_filename)
{
  std::unique_lock lock(_impl->global_mutex);

  if (verbose() >= 1)
    log::info() << "Saving cache graph " << cache_graph_filename << "...";
  _impl->cache.save_graph(cache_graph_filename);
}

std::tuple<std::size_t /*succeeded_job_count*/,
           std::size_t /*failed_job_count*/>
resource_compiler::update()
{
  std::unique_lock lock(_impl->global_mutex);

  std::atomic_size_t succeeded_job_count = ATOMIC_VAR_INIT(0);
  std::atomic_size_t failed_job_count = ATOMIC_VAR_INIT(0);

  // Try to match each file found in input_path with one of the available rules.
  std::uint32_t current_pass = 0;
  for (auto input_iterator =
         fs::recursive_directory_iterator(_impl->input_path);
       input_iterator != fs::recursive_directory_iterator(); ++input_iterator)
  {
    const auto& file_path = input_iterator->path();
    if (fs::is_regular_file(file_path))
    {
      if (auto* file = _impl->add_file(file_path, current_pass);
          file != nullptr)
      {
        _impl->match_file(*file, current_pass);
      }
    }
  }

  // Loop until there is no more work to do.
  /// ToDo: Add method to quit this loop early.
  for (;;)
  {
    std::vector<std::unique_ptr<job_description>> jobs;
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

    // Spawn a task in parallel for each job in the current pass.
    std::size_t modified_jobs_count = 0;
    for (const auto& job : jobs)
    {
      if (job->flags.test(entity_flag::modified))
        ++modified_jobs_count;
      else
        job->mark_as_used();
    }
    log::info() << "Queuing " << modified_jobs_count
                << " modified jobs(s) in pass " << current_pass << " (skipping "
                << (jobs.size() - modified_jobs_count)
                << " unmodified job(s))...";
    std::vector<task::future<bool>> task_results;
    task_results.reserve(modified_jobs_count);
    auto process_job = [&](rc::job_description* job) -> bool {
      /// ToDo: Change job parameter of reference type.
      BOOST_ASSERT(job != nullptr);
      BOOST_ASSERT(job->rule != nullptr);
      BOOST_ASSERT(job->rule->action != nullptr);
      if (job == nullptr || job->rule == nullptr ||
          job->rule->action == nullptr)
      {
        job->flags.set(entity_flag::failed);
        ++failed_job_count;
        return false;
      }

      // There should be no tasks for unmodified jobs.
      BOOST_ASSERT(job->flags.test(entity_flag::modified));

      bool result = false;
      try
      {
        result = job->rule->action->impl->process(*_impl, *job);
        if (verbose() >= 1)
        {
          log::info line;
          bool first = true;
          line << "(";
          for (const auto& [input_slot_index, input] : job->inputs)
          {
            if (first)
              first = false;
            else
              line << ", ";
            line << input_slot_index << ": " << input->file->generic_string;
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
          for (const auto& [input_slot_index, input] : job->inputs)
          {
            if (first)
              first = false;
            else
              line << ", ";
            line << input_slot_index << ": " << input->file->generic_string;
          }
          line << ") -> failed";
        }
        log::exception() << boost::diagnostic_information(e);
      }

      if (result)
      {
        job->flags.reset(entity_flag::failed);
        ++succeeded_job_count;
        job->mark_as_used();

        // Push all output files into the pipeline.
        for (auto* output_file : job->outputs)
        {
          BOOST_ASSERT(output_file != nullptr);
          _impl->match_file(*output_file, job->rule->pass);
        }
        return true;
      }
      else
      {
        job->flags.set(entity_flag::failed);
        log::error() << "A job using rule " << job->rule->id << " failed.";
        return false;
      }
    };

    {
      // Spawn all jobs serially that don't support multithreading.
      // Use an empty dummy task to ease initialization of serial_result.
      auto serial_result = task::async([]() -> bool { return true; });
      bool has_serial_job = false;
      for (const auto& job : jobs)
      {
        if (!job->flags.test(entity_flag::modified) ||
            job->rule->action->impl->support_multithreading())
        {
          continue;
        }

        serial_result = std::move(serial_result)
                          .then(
                            [&](task::future<bool> previous_result,
                                rc::job_description* job) -> bool {
                              return previous_result.get() && process_job(job);
                            },
                            job.get());
        has_serial_job = true;
      }
      if (has_serial_job)
      {
        // Add the last serial_result to the list of futures to wait on.
        task_results.emplace_back(std::move(serial_result));
      }
    }

    // Spawn all jobs in parallel that do support multithreading.
    for (const auto& job : jobs)
    {
      if (!job->flags.test(entity_flag::modified) ||
          !job->rule->action->impl->support_multithreading())
      {
        continue;
      }

      task_results.emplace_back(task::async(process_job, job.get()));
    }

    // Wait until all tasks completed.
    task::when_all(begin(task_results), end(task_results)).get();

    // Cache all succeeded jobs.
    for (auto& job : jobs)
    {
      if (!job->flags.test(entity_flag::failed))
        _impl->cache.add_job(std::move(job));
    }
  }
  return {succeeded_job_count, failed_job_count};
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
