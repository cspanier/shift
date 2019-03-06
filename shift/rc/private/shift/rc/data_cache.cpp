#include "shift/rc/data_cache.hpp"
#include "shift/rc/resource_compiler_impl.hpp"
#include <shift/parser/json/json.hpp>
#include <shift/log/log.hpp>
#include <filesystem>
#include <fstream>

namespace shift::rc
{
data_cache::data_cache(resource_compiler_impl& impl) : _impl(&impl)
{
}

data_cache::~data_cache()
{
  _impl = nullptr;
}

void data_cache::register_action(std::string name, action_version version,
                                 action_base& impl)
{
  BOOST_ASSERT(_rules.empty() && _files.empty() && _jobs.empty());

  auto new_action = std::make_unique<action_description>(name, version, &impl,
                                                         entity_flag::modified);
  std::string_view key = new_action->name;
  _actions.insert_or_assign(key, std::move(new_action));
}

bool data_cache::load(const std::filesystem::path& cache_filename)
{
  using namespace shift::parser;

  BOOST_ASSERT(!_actions.empty());

  if (!fs::exists(cache_filename))
    return false;

  json::value root;
  if (std::ifstream file{cache_filename.generic_string(),
                         std::ios_base::in | std::ios_base::binary};
      file.is_open())
  {
    try
    {
      file >> root;
    }
    catch (json::parse_error&)
    {
      // Simply ignore any JSON parse error.
      return false;
    }
  }
  else
    return false;

  auto& root_object = json::get<json::object>(root);

  // Read all cached actions.
  if (const auto* cached_actions =
        json::get_if<json::object>(root_object, "actions");
      cached_actions != nullptr)
  {
    for (const auto& cached_action : *cached_actions)
    {
      auto& cached_action_name = cached_action.first;
      if (const auto* cached_version =
            json::get_if<std::string>(&cached_action.second);
          cached_version != nullptr)
      {
        if (auto action_iter = _actions.find(cached_action_name);
            action_iter != _actions.end())
        {
          auto& action = *action_iter->second;
          if (action.version == *cached_version)
          {
            // The cached action has the same version, so remove the modified
            // flag.
            action.flags.reset(entity_flag::modified);
            action.flags.set(entity_flag::used);
          }
          else
          {
            // The cached action has a different version than the built-in one.
            // Keep the modified flag because we need to re-run all associated
            // jobs. This happens if the action's implementation changed.
          }
        }
        else
        {
          // The cached action does not exist any more. This happens if the
          // action was removed from code or if we're running an older version
          // of the resource compiler using a more recent cache file. In this
          // case we simply ignore the cached action. As a consequence, rules
          // loaded below still using the non-existing action will also fail to
          // load.
        }
      }
    }
  }

  // Read all cached rules.
  if (const auto* cached_rules =
        json::get_if<json::object>(root_object, "rules");
      cached_rules != nullptr)
  {
    for (const auto& cached_rule : *cached_rules)
    {
      if (const auto* rule_object =
            json::get_if<json::object>(&cached_rule.second);
          rule_object != nullptr)
      {
        auto new_rule = std::make_unique<rule_description>();
        new_rule->id = cached_rule.first;
        if (const auto* pass = json::get_if<double>(*rule_object, "pass");
            pass != nullptr)
        {
          new_rule->pass = static_cast<std::uint32_t>(*pass);
        }
        else
          continue;

        if (const auto* action_name =
              json::get_if<std::string>(*rule_object, "action");
            action_name != nullptr)
        {
          auto action_iter = _actions.find(*action_name);
          // Skip this rule because the associated action does not exist. This
          // may happen when the action is removed from code. Any cached jobs
          // associated with this action will also fail to load.
          if (action_iter == _actions.end())
            continue;
          new_rule->action = action_iter->second.get();
          if (new_rule->action->flags.test(entity_flag::modified))
            new_rule->flags.set(entity_flag::modified);
        }
        else
          continue;

        // new_rule->action->flags

        if (const auto* path = json::get_if<std::string>(*rule_object, "path");
            path != nullptr)
        {
          new_rule->path = *path;
        }
        else
          continue;

        if (const auto* inputs =
              json::get_if<json::object>(*rule_object, "inputs");
            inputs != nullptr)
        {
          for (const auto& input_iter : *inputs)
          {
            if (const auto* input =
                  json::get_if<std::string>(&input_iter.second);
                input != nullptr)
            {
              new_rule->inputs.insert_or_assign(
                input_iter.first,
                rule_input{*input,
                           std::regex{*input, std::regex_constants::ECMAScript |
                                                std::regex_constants::icase}});
            }
          }
        }
        else
          continue;

        if (const auto* group_by_array =
              json::get_if<json::array>(*rule_object, "group-by");
            group_by_array != nullptr)
        {
          for (const auto& group_by : *group_by_array)
          {
            if (auto* group_by_value = json::get_if<double>(&group_by))
            {
              new_rule->group_by.insert(
                static_cast<std::size_t>(*group_by_value));
            }
          }
        }
        else
          continue;

        if (const auto* outputs =
              json::get_if<json::object>(*rule_object, "outputs");
            outputs != nullptr)
        {
          for (const auto& output_iter : *outputs)
          {
            if (const auto* output =
                  json::get_if<std::string>(&output_iter.second))
            {
              new_rule->outputs.insert_or_assign(output_iter.first, *output);
            }
          }
        }
        else
          continue;

        new_rule->options =
          *json::get_if<json::object>(*rule_object, "options");

        std::string_view id = new_rule->id;
        if (!_rules.insert_or_assign(id, std::move(new_rule)).second)
        {
          log::warning()
            << R"(Found non-unique rule id ")" << id
            << R"(" in cache. Either one of the rules won't be loaded.)";
          continue;
        }
      }
    }
  }

  // Read all cached files.
  if (const auto* cached_files =
        json::get_if<json::object>(root_object, "files");
      cached_files != nullptr)
  {
    for (const auto& cached_file : *cached_files)
    {
      if (const auto* cached_file_object =
            json::get_if<json::object>(&cached_file.second);
          cached_file_object != nullptr)
      {
        auto new_file =
          std::make_unique<file_description>(fs::path{cached_file.first});

        if (json::has(*cached_file_object, "write-time") &&
            (json::get_if<double>(&cached_file_object->at("write-time")) !=
             nullptr))
        {
          new_file->last_write_time =
            std::chrono::system_clock::from_time_t(static_cast<time_t>(
              json::get<double>(cached_file_object->at("write-time"))));
        }

        if (json::has(*cached_file_object, "pass") &&
            (json::get_if<double>(&cached_file_object->at("pass")) != nullptr))
        {
          new_file->pass = static_cast<std::uint32_t>(
            json::get<double>(cached_file_object->at("pass")));
        }

        std::string_view file_key = new_file->generic_string;
        if (!_files.try_emplace(file_key, std::move(new_file)).second)
        {
          /// ToDo: Log warning about non-unique file path.
          continue;
        }
      }
    }

    // // Resolve alias references in a second pass to make sure that all
    // files
    // // are present.
    // for (const auto& cached_file : *cached_files)
    // {
    //   if (const auto* cached_file_object =
    //         json::get_if<json::object>(&cached_file.second))
    //   {
    //     if (json::has(*cached_file_object, "alias") &&
    //         (json::get_if<std::string>(&cached_file_object->at("alias")) !=
    //          nullptr))
    //     {
    //       if (auto* file = get_file(fs::path{cached_file.first}))
    //       {
    //         file->alias = get_file(fs::path{
    //           json::get<std::string>(cached_file_object->at("alias"))});
    //       }
    //     }
    //   }
    // }
  }

  // Read all jobs.
  if (const auto* cached_jobs = json::get_if<json::object>(root_object, "jobs");
      cached_jobs != nullptr)
  {
    for (const auto& cached_job : *cached_jobs)
    {
      if (const auto* job_object =
            json::get_if<json::object>(&cached_job.second))
      {
        auto new_job = std::make_unique<job_description>();

        if (!json::has(*job_object, "rule") ||
            (json::get_if<std::string>(&job_object->at("rule")) == nullptr))
        {
          continue;
        }
        auto rule_id = json::get<std::string>(job_object->at("rule"));

        if (auto rule_iter = _rules.find(rule_id); rule_iter != _rules.end())
          new_job->rule = rule_iter->second.get();
        else
        {
          /// ToDo: Log warning about missing rule reference in cached job.
          log::warning()
            << "Some cached job references the non-existing rule \"" << rule_id
            << "\".";
          continue;
        }

        if (!json::has(*job_object, "inputs") ||
            (json::get_if<json::array>(&job_object->at("inputs")) == nullptr))
        {
          continue;
        }
        bool input_files_missing = false;
        auto inputs_array = json::get<json::array>(job_object->at("inputs"));
        for (const auto& input_value : inputs_array)
        {
          const auto* input_name = json::get_if<std::string>(&input_value);
          if (input_name == nullptr)
          {
            log::warning()
              << "The inputs array of some cached job contains "
                 "a value that is not of type string, which will be ignored.";
            input_files_missing = true;
            break;
          }

          auto match = std::make_unique<input_match>();
          match->file = get_file(std::string_view{*input_name});
          if (match->file == nullptr)
          {
            input_files_missing = true;
            break;
          }
          match->file->flags |= entity_flag::used;
          std::size_t slot_index = 0;
          for (auto input_iter = new_job->rule->inputs.begin();
               input_iter != new_job->rule->inputs.end();
               ++input_iter, ++slot_index)
          {
            if (std::regex_search(match->file->generic_string,
                                  match->match_results,
                                  input_iter->second.pattern))
            {
              match->slot = input_iter;
              match->slot_index = slot_index;
              new_job->inputs.insert({slot_index, std::move(match)});
              break;
            }
          }
          if (match)
          {
            // If match was not moved to the new job's inputs vector
            // something is wrong.
            input_files_missing = true;
            break;
          }
        }

        bool output_files_missing = false;
        if (!json::has(*job_object, "outputs") ||
            (json::get_if<json::array>(&job_object->at("outputs")) == nullptr))
        {
          continue;
        }
        auto outputs_array = json::get<json::array>(job_object->at("outputs"));
        for (const auto& output_value : outputs_array)
        {
          const auto* output_name = json::get_if<std::string>(&output_value);
          if (output_value == nullptr)
          {
            log::warning()
              << "The outputs array of some cached job contains a value that "
                 "is not of type string, which will be ignored.";
            output_files_missing = true;
            break;
          }
          auto* output = get_file(std::string_view{*output_name});
          if (output == nullptr)
          {
            output_files_missing = true;
            break;
          }
          output->flags |= entity_flag::used;
          new_job->outputs.insert(output);
        }

        if (input_files_missing || output_files_missing)
          continue;

        std::size_t job_id;
        if (std::stringstream job_id_stream(cached_job.first);
            !(job_id_stream >> job_id))
        {
          /// ToDo: Warn about bad job id.
          continue;
        }
        if (job_id != std::hash<job_description>()(*new_job))
        {
          /// ToDo: Warn about bad job id.
          continue;
        }
        new_job->id = job_id;
        if (!_jobs.insert_or_assign(job_id, std::move(new_job)).second)
        {
          /// ToDo: Warn about non-unique job id. This should not be
          /// possible.
        }
      }
    }
  }

  // Read file source references in a second pass.
  if (const auto* cached_files =
        json::get_if<json::object>(root_object, "files");
      cached_files != nullptr)
  {
    for (const auto& cached_file : *cached_files)
    {
      if (const auto* cached_file_object =
            json::get_if<json::object>(&cached_file.second);
          cached_file_object != nullptr)
      {
        auto* file = get_file(fs::path{cached_file.first});
        BOOST_ASSERT(file != nullptr);
        if (file == nullptr)
          continue;

        if (const auto* source_id =
              json::get_if<double>(*cached_file_object, "source");
            source_id != nullptr)
        {
          if (auto job_iter = _jobs.find(static_cast<std::size_t>(*source_id));
              job_iter != _jobs.end())
          {
            file->source = job_iter->second.get();
          }
        }
      }
    }
  }

  // Check for unreferenced or invalid nodes.
  auto file_iterator = _files.begin();
  while (file_iterator != _files.end())
  {
    if (!file_iterator->second->flags.test(entity_flag::used))
      file_iterator = _files.erase(file_iterator);
    else
      ++file_iterator;
  }

  return true;
}

void data_cache::save(const std::filesystem::path& cache_filename) const
{
  using namespace shift::parser;

  json::object root;

  // Cache all action descriptions.
  auto& actions_object =
    json::get<json::object>(root["actions"] = json::object{});
  for (const auto& [action_name, action] : _actions)
    actions_object[std::string{action_name}] = action->version;

  // Cache all rules.
  auto& rules_object = json::get<json::object>(root["rules"] = json::object{});
  for (const auto& rule : _impl->rules)
  {
    if (rule->action == nullptr)
      continue;

    auto& rule_object =
      json::get<json::object>(rules_object[rule->id] = json::object{});

    rule_object["pass"] = static_cast<double>(rule->pass);
    rule_object["action"] = rule->action->name;
    rule_object["path"] = rule->path.generic_string();
    auto& inputs_object =
      json::get<json::object>(rule_object["inputs"] = json::object{});
    for (const auto& input : rule->inputs)
      inputs_object.insert_or_assign(input.first, input.second.source);

    auto& group_by_array =
      json::get<json::array>(rule_object["group-by"] = json::array{});
    for (const auto value : rule->group_by)
      group_by_array.emplace_back(static_cast<double>(value));

    auto& outputs_object =
      json::get<json::object>(rule_object["outputs"] = json::object{});
    for (const auto& output : rule->outputs)
      outputs_object.insert_or_assign(output.first, output.second);

    rule_object["options"] = rule->options;
  }

  // Cache all jobs.
  auto& jobs_object = json::get<json::object>(root["jobs"] = json::object{});
  for (const auto& [job_id, job] : _jobs)
  {
    // Don't cache jobs that failed execution.
    if (job->flags.test(entity_flag::failed))
      continue;

    json::object job_object;
    job_object["rule"] = job->rule->id;

    auto& inputs_array =
      json::get<json::array>(job_object["inputs"] = json::array{});
    for (const auto& [input_slot_index, input] : job->inputs)
      inputs_array.emplace_back(input->file->generic_string);

    auto& outputs_array =
      json::get<json::array>(job_object["outputs"] = json::array{});
    for (const auto* output : job->outputs)
      outputs_array.emplace_back(output->path.generic_string());

    jobs_object.insert_or_assign(std::to_string(job_id), std::move(job_object));
  }

  // Cache all existing files.
  auto& files_object = json::get<json::object>(root["files"] = json::object{});
  for (auto& file : _impl->files)
  {
    if (!(file.second->flags & entity_flag::exists) ||
        !(file.second->flags & entity_flag::used))
    {
      continue;
    }

    auto& file_object = json::get<json::object>(
      files_object[file.first.generic_string()] = json::object{});
    /// ToDo: This hack is required because some files magically change their
    /// time-stamp when it is queried right after the files were closed.
#if defined(_MSC_VER)
    // This ugly hack attempts to convert a std::filesystem::file_time_type to
    // std::chrono::system_clock::time_point. It likely produces wrong results
    // because there is no guarantee that both clocks have the same epoch.
    // C++20 will fix this issue and make clocks convertible.
    auto last_write_time = fs::last_write_time(file.first);
    file.second->last_write_time =
      std::chrono::system_clock::time_point(std::chrono::system_clock::duration(
        decltype(last_write_time)::clock::duration::rep{
          last_write_time.time_since_epoch().count()}));
#else
    file.second->last_write_time = fs::last_write_time(file.first);
#endif
    file_object["write-time"] = static_cast<double>(
      std::chrono::system_clock::to_time_t(file.second->last_write_time));
    if (file.second->pass > 0)
      file_object["pass"] = static_cast<double>(file.second->pass);
    if ((file.second->alias != nullptr) &&
        (file.second->alias->flags & entity_flag::exists) &&
        (file.second->alias->flags & entity_flag::used))
    {
      file_object["alias"] = file.second->alias->generic_string;
    }
    if (file.second->source != nullptr)
      file_object["source"] = static_cast<double>(file.second->source->id);
  }

  std::ofstream file{cache_filename.generic_string(), std::ios_base::out |
                                                        std::ios_base::binary |
                                                        std::ios_base::trunc};
  if (!file.is_open())
    return;

  file << core::indent_character(' ') << core::indent_width(2);
  file << root;
  file.close();
}

void data_cache::save_graph(const fs::path& graph_filename) const
{
  std::ofstream file{graph_filename.generic_string(), std::ios_base::out |
                                                        std::ios_base::binary |
                                                        std::ios_base::trunc};
  if (!file.is_open())
    return;

  static constexpr char br = '\n';

  file << R"(digraph Cache {
  rankdir=LR;
  size="8,5"
  node [shape=box]
)";

  for (const auto& [job_id, job] : _jobs)
  {
    // Don't write jobs that failed execution.
    if (job->flags.test(entity_flag::failed))
      continue;

    for (const auto& [input_slot_index, input] : job->inputs)
    {
      file << R"(  ")" << input->file->generic_string << R"(" -> ")"
           << job->rule->id << '#' << job_id << R"(";)" << br;
    }

    for (const auto* output : job->outputs)
    {
      file << R"(  ")" << job->rule->id << '#' << job_id << R"(" -> ")"
           << output->path.generic_string() << R"(";)" << br;
    }
  }

  file << R"(})" << br;
  file.close();
}

action_description* data_cache::find_action(std::string_view name) const
{
  if (auto action_iter = _actions.find(name); action_iter != _actions.end())
    return action_iter->second.get();
  else
    return nullptr;
}

file_description* data_cache::get_file(const fs::path& file_path)
{
  return get_file(std::string_view{file_path.generic_string()});
}

file_description* data_cache::get_file(std::string_view file_path)
{
  auto cached_file_iter = _files.find(file_path);
  if (cached_file_iter != _files.end())
    return cached_file_iter->second.get();
  else
    return nullptr;
}

const job_description* data_cache::get_job(const job_description& job) const
{
  if (auto job_iter = _jobs.find(std::hash<job_description>()(job));
      job_iter != _jobs.end())
  {
    return job_iter->second.get();
  }
  else
    return nullptr;
}

void data_cache::add_job(std::unique_ptr<job_description> job)
{
  auto id = std::hash<job_description>()(*job);
  _jobs.insert_or_assign(id, std::move(job));
}

rule_description& data_cache::get_rule(const rule_create_info& create_info)
{
  auto rule_iter = _rules.find(create_info.id);
  if (rule_iter != _rules.end())
    return *rule_iter->second;
  else
  {
    auto new_rule = std::make_unique<rule_description>(create_info);
    new_rule->flags.set(entity_flag::modified);
    std::string_view id = new_rule->id;
    return *_rules.insert_or_assign(id, std::move(new_rule)).first->second;
  }
}

bool data_cache::is_modified(const job_description& job) const
{
  if (job.rule->action->flags.test(entity_flag::modified))
  {
    // log::debug() << "Action " << job.rule->action->name()
    //             << " is modified.";
    return true;
  }
  if (job.rule->flags.test(entity_flag::modified))
  {
    // log::debug() << "Rule " << job.rule->id << " is modified.";
    return true;
  }
  for (const auto& [input_slot_index, input] : job.inputs)
  {
    if (is_modified(*input->file))
    {
      // log::debug() << "Input " << input->file->path << " is modified.";
      return true;
    }
  }
  for (const auto* output_file : job.outputs)
  {
    if (is_modified(*output_file))
    {
      // log::debug() << "Output " << output_file->path << " is modified.";
      return true;
    }
  }
  return false;
}

bool data_cache::is_modified(const file_description& file) const
{
  auto cached_file_iter = _files.find(file.generic_string);
  if (cached_file_iter != _files.end())
  {
    if (file.last_write_time != cached_file_iter->second->last_write_time)
    {
      // log::debug() << "File " << file.path << " modified because "
      //             << file.last_write_time
      //             << " != " << cached_file_iter->second->last_write_time;
      return true;
    }
    else
      return false;
  }
  // log::debug() << "File " << file.path
  //             << " modified because it cannot be located in our cache.";
  return true;
}
}
