#include "shift/rc/data_cache.h"
#include "shift/rc/resource_compiler_impl.h"
#include <shift/parser/json/json.h>
#include <boost/filesystem/operations.hpp>
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

bool data_cache::load(const boost::filesystem::path& cache_filename)
{
  using namespace shift::parser;

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
  auto actions_iter = root_object.find("actions");
  if (actions_iter != root_object.end())
  {
    if (const auto* cached_actions =
          json::get_if<json::object>(&actions_iter->second);
        cached_actions != nullptr)
    {
      for (const auto& cached_action : *cached_actions)
      {
        if (const auto* cached_version =
              json::get_if<std::string>(&cached_action.second);
            cached_version != nullptr)
        {
          auto action = std::make_unique<action_description>(
            cached_action.first, *cached_version);
          std::string_view action_name = action->name;
          _actions.try_emplace(action_name, std::move(action));

          // Check if the cached action still exists and has the same version.
          // We need to differenciate between three cases:
          // 1. The cached action is available and has the same version.
          // 2. The cached action is available but has a different version.
          //    This happens if the implementation and thus the version of the
          //    action changes. In this case we need to re-run all jobs using
          //    this action by marking the action as modified.
          // 3. The cached action is not available.
          //    This happens if the action was removed from code or if we're
          //    running an older version of the resource compiler using a more
          //    recent cache file.
          //    In this case we simply ignore the cached action. As a
          //    consequence, rules loaded below still using the non-existing
          //    action will also fail to load.
          for (auto& [action_name, action] : _impl->actions)
          {
            if (action_name == cached_action.first)
            {
              action->modified(action->modified() ||
                               action->compare_version(*cached_version));
              break;
            }
          }
        }
      }
    }
  }

  // Read all cached rules.
  auto rules_iter = root_object.find("rules");
  if (rules_iter != root_object.end())
  {
    if (const auto* cached_rules =
          json::get_if<json::object>(&rules_iter->second))
    {
      for (const auto& cached_rule : *cached_rules)
      {
        if (const auto* rule_object =
              json::get_if<json::object>(&cached_rule.second))
        {
          rule_description new_rule;
          new_rule.id = cached_rule.first;
          if (!json::has(*rule_object, "pass") ||
              (json::get_if<double>(&rule_object->at("pass")) == nullptr))
          {
            continue;
          }
          new_rule.pass = static_cast<std::uint32_t>(
            json::get<double>(rule_object->at("pass")));

          if (!json::has(*rule_object, "action") ||
              (json::get_if<std::string>(&rule_object->at("action")) ==
               nullptr))
          {
            continue;
          }
          auto action_name = json::get<std::string>(rule_object->at("action"));
          auto action_iter = _impl->actions.find(action_name);
          // Skip this rule of the associated action does not exist. This may
          // happen when the action is removed from code. Any cached jobs
          // associated with this action will also fail to load.
          if (action_iter == _impl->actions.end())
            continue;
          new_rule.action = action_iter->second;

          if (!json::has(*rule_object, "path") ||
              (json::get_if<std::string>(&rule_object->at("path")) == nullptr))
          {
            continue;
          }
          new_rule.rule_path = json::get<std::string>(rule_object->at("path"));

          if (!json::has(*rule_object, "inputs") ||
              (json::get_if<json::object>(&rule_object->at("inputs")) ==
               nullptr))
          {
            continue;
          }
          const auto& inputs_object =
            json::get<json::object>(rule_object->at("inputs"));
          for (const auto& input_iter : inputs_object)
          {
            if (const auto* input =
                  json::get_if<std::string>(&input_iter.second))
            {
              new_rule.inputs.insert_or_assign(
                input_iter.first,
                rule_input{*input,
                           std::regex{*input, std::regex_constants::ECMAScript |
                                                std::regex_constants::icase}});
            }
          }

          if (!json::has(*rule_object, "group-by") ||
              (json::get_if<json::array>(&rule_object->at("group-by")) ==
               nullptr))
          {
            continue;
          }
          const auto& group_by_array =
            json::get<json::array>(rule_object->at("group-by"));
          for (const auto& group_by : group_by_array)
          {
            if (auto* group_by_value = json::get_if<double>(&group_by))
            {
              new_rule.group_by.insert(
                static_cast<std::size_t>(*group_by_value));
            }
          }

          if (!json::has(*rule_object, "outputs") ||
              (json::get_if<json::object>(&rule_object->at("outputs")) ==
               nullptr))
          {
            continue;
          }
          const auto& outputs_object =
            json::get<json::object>(rule_object->at("outputs"));
          for (const auto& output_iter : outputs_object)
          {
            if (const auto* output =
                  json::get_if<std::string>(&output_iter.second))
            {
              new_rule.outputs.insert_or_assign(output_iter.first, *output);
            }
          }

          if (!json::has(*rule_object, "options") ||
              (json::get_if<json::object>(&rule_object->at("options")) ==
               nullptr))
          {
            continue;
          }
          new_rule.options =
            json::get<json::object>(rule_object->at("options"));

          // Search for the rule among the actually existing ones.
          auto existing_rule_iter =
            std::find_if(_impl->rules.begin(), _impl->rules.end(),
                         [&](const auto& existing_rule) {
                           return existing_rule->id == new_rule.id;
                         });
          if (existing_rule_iter == _impl->rules.end())
          {
            // There is no rule with the given id any more, so it must have been
            // deleted since the last RC invocation. Ignore the cached rule in
            // this case.
            continue;
          }
          auto* existing_rule = existing_rule_iter->get();
          if (*existing_rule == new_rule)
          {
            // The cached rule matches the previously added one, thus remove the
            // modified flag.
            existing_rule->modified = false;
          }
        }
      }
    }
  }

  // Read all cached files.
  auto files_iter = root_object.find("files");
  if (files_iter != root_object.end())
  {
    if (const auto* cached_files =
          json::get_if<json::object>(&files_iter->second))
    {
      for (const auto& cached_file : *cached_files)
      {
        if (const auto* cached_file_object =
              json::get_if<json::object>(&cached_file.second))
        {
          fs::path file_path{cached_file.first};
          auto file_iter =
            _files
              .insert_or_assign(file_path,
                                std::make_unique<file_stats>(file_path))
              .first;
          auto* file = file_iter->second.get();

          if (json::has(*cached_file_object, "write-time") &&
              (json::get_if<double>(&cached_file_object->at("write-time")) !=
               nullptr))
          {
            file->last_write_time = static_cast<time_t>(
              json::get<double>(cached_file_object->at("write-time")));
          }

          if (json::has(*cached_file_object, "pass") &&
              (json::get_if<double>(&cached_file_object->at("pass")) !=
               nullptr))
          {
            file->pass = static_cast<std::uint32_t>(
              json::get<double>(cached_file_object->at("pass")));
          }
        }
      }

      // Resolve alias references in a second pass to make sure that all files
      // are present.
      for (const auto& cached_file : *cached_files)
      {
        if (const auto* cached_file_object =
              json::get_if<json::object>(&cached_file.second))
        {
          if (json::has(*cached_file_object, "alias") &&
              (json::get_if<std::string>(&cached_file_object->at("alias")) !=
               nullptr))
          {
            if (auto* file = get_file(fs::path{cached_file.first}))
            {
              file->alias = get_file(fs::path{
                json::get<std::string>(cached_file_object->at("alias"))});
            }
          }
        }
      }
    }
  }

  // Read all jobs.
  auto jobs_iter = root_object.find("jobs");
  if (jobs_iter != root_object.end())
  {
    if (const auto* cached_jobs = json::get_if<json::array>(&jobs_iter->second))
    {
      for (const auto& cached_job : *cached_jobs)
      {
        if (const auto* job_object = json::get_if<json::object>(&cached_job))
        {
          auto new_job = std::make_unique<job_description>();

          if (!json::has(*job_object, "rule") ||
              (json::get_if<std::string>(&job_object->at("rule")) == nullptr))
          {
            continue;
          }
          auto rule_id = json::get<std::string>(job_object->at("rule"));
          auto rule_iter =
            std::find_if(_impl->rules.begin(), _impl->rules.end(),
                         [&](const auto& rule) { return rule->id == rule_id; });
          if (rule_iter == _impl->rules.end())
            continue;
          auto* rule = rule_iter->get();
          new_job->matching_rule = rule_iter->get();

          if (!json::has(*job_object, "inputs") ||
              (json::get_if<json::array>(&job_object->at("inputs")) == nullptr))
          {
            continue;
          }
          bool input_files_missing = false;
          auto inputs_array = json::get<json::array>(job_object->at("inputs"));
          for (const auto& input_value : inputs_array)
          {
            if (const auto* input_name =
                  json::get_if<std::string>(&input_value))
            {
              auto* input = get_file(fs::path{*input_name});
              if (input == nullptr)
              {
                input_files_missing = true;
                break;
              }
              auto match = std::make_unique<input_match>();
              match->slot_index = 0;
              match->file = input;
              for (auto input_iter = rule->inputs.begin();
                   input_iter != rule->inputs.end();
                   ++input_iter, ++match->slot_index)
              {
                if (std::regex_search(match->file->generic_string,
                                      match->match_results,
                                      input_iter->second.pattern))
                {
                  match->slot = input_iter;
                  new_job->inputs.push_back(std::move(match));
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
          }

          bool output_files_missing = false;
          if (!json::has(*job_object, "outputs") ||
              (json::get_if<json::array>(&job_object->at("outputs")) ==
               nullptr))
          {
            continue;
          }
          auto outputs_array =
            json::get<json::array>(job_object->at("outputs"));
          for (const auto& output_value : outputs_array)
          {
            if (const auto* output_name =
                  json::get_if<std::string>(&output_value))
            {
              auto* output = get_file(fs::path{*output_name});
              if (output == nullptr)
              {
                output_files_missing = true;
                break;
              }
              new_job->outputs.insert(output);
            }
          }

          if (!input_files_missing && !output_files_missing)
            _jobs.insert(std::move(new_job));
        }
      }
    }
  }

  return true;
}

void data_cache::save(const boost::filesystem::path& cache_filename) const
{
  using namespace shift::parser;

  json::object root;

  // Cache all action descriptions.
  auto& actions_object =
    json::get<json::object>(root["actions"] = json::object{});
  for (const auto& [action_name, action] : _impl->actions)
    actions_object[action_name] = action->version();

  // Cache all rules.
  auto& rules_object = json::get<json::object>(root["rules"] = json::object{});
  for (const auto& rule : _impl->rules)
  {
    if (rule->action == nullptr)
      continue;

    auto& rule_object =
      json::get<json::object>(rules_object[rule->id] = json::object{});

    rule_object["pass"] = static_cast<double>(rule->pass);
    rule_object["action"] = rule->action->name();
    rule_object["path"] = rule->rule_path.generic_string();
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
  auto& jobs_array = json::get<json::array>(root["jobs"] = json::array{});
  for (const auto& job : _impl->jobs)
  {
    // Don't cache jobs that failed execution.
    if (job->flags.test(entity_flag::failed))
      continue;

    json::object job_object;
    job_object["rule"] = job->matching_rule->id;

    auto& inputs_array =
      json::get<json::array>(job_object["inputs"] = json::array{});
    for (const auto& input : job->inputs)
      inputs_array.emplace_back(input->file->generic_string);

    auto& outputs_array =
      json::get<json::array>(job_object["outputs"] = json::array{});
    for (const auto* output : job->outputs)
      outputs_array.emplace_back(output->path.generic_string());

    jobs_array.emplace_back(std::move(job_object));
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
    file.second->last_write_time = fs::last_write_time(file.first);
    file_object["write-time"] =
      static_cast<double>(file.second->last_write_time);
    if (file.second->pass > 0)
      file_object["pass"] = static_cast<double>(file.second->pass);
    if ((file.second->alias != nullptr) &&
        (file.second->alias->flags & entity_flag::exists) &&
        (file.second->alias->flags & entity_flag::used))
    {
      file_object["alias"] = file.second->alias->generic_string;
    }
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

  file << R"(digraph Cache {)" << br;
  file << R"(  rankdir=LR;)" << br;
  file << R"(  size="8,5")" << br;

  std::uint32_t job_id = 1;
  for (const auto& job : _impl->jobs)
  {
    // Don't write jobs that failed execution.
    if (job->flags.test(entity_flag::failed))
      continue;

    for (const auto& input : job->inputs)
    {
      file << R"(  ")" << input->file->generic_string << R"(" -> ")"
           << job->matching_rule->id << '#' << job_id << R"(";)" << br;
    }

    for (const auto* output : job->outputs)
    {
      file << R"(  ")" << job->matching_rule->id << '#' << job_id << R"(" -> ")"
           << output->path.generic_string() << R"(";)" << br;
    }

    ++job_id;
  }

  file << R"(})" << br;
  file.close();
}

file_stats* data_cache::get_file(const fs::path& file_path)
{
  auto cached_file_iter = _files.find(file_path);
  if (cached_file_iter != _files.end())
    return cached_file_iter->second.get();
  else
    return nullptr;
}

const job_description* data_cache::get_job(const job_description& job) const
{
  for (const auto& cached_job : _jobs)
  {
    if (*cached_job == job)
      return cached_job.get();
  }
  return nullptr;
}

bool data_cache::is_modified(const job_description& job) const
{
  if (job.matching_rule->action->modified())
  {
    // log::debug() << "Action " << job.matching_rule->action->name()
    //             << " is modified.";
    return true;
  }
  if (job.matching_rule->modified)
  {
    // log::debug() << "Rule " << job.matching_rule->id << " is modified.";
    return true;
  }
  for (const auto& input : job.inputs)
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

bool data_cache::is_modified(const file_stats& file) const
{
  auto cached_file_iter = _files.find(file.path);
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
