#include "shift/rc/resource_compiler_impl.h"
#include "shift/rc/types.h"
#include "shift/rc/action_group_resources.h"
#include "shift/rc/action_image_import.h"
#include "shift/rc/action_mesh_tootle.h"
#include "shift/rc/action_mesh_import_ply.h"
#include "shift/rc/action_mesh_export_obj.h"
#include "shift/rc/action_font_import_ttf.h"
#include "shift/rc/action_shader_compile.h"
#include "shift/rc/action_scene_compile.h"
#include "shift/rc/action_scene_import_gltf.h"
#include "shift/rc/action_scene_import_pbrt.h"
#include <shift/resource/repository.h>
#include <shift/parser/json/json.h>
#include <shift/log/log.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <chrono>
#include <map>
#include <set>
#include <iostream>

namespace shift::rc
{
static std::string regex_escape(const std::string& input)
{
  static constexpr std::array<char, 11> special_characters = {
    {'+', '-', '*', '.', '[', ']', '(', ')', '$', '^', '\\'}};

  std::stringstream ss;
  for (auto c : input)
  {
    if (std::find(special_characters.begin(), special_characters.end(), c) !=
        special_characters.end())
    {
      ss << '\\';
    }
    ss << c;
  }
  return ss.str();
};

resource_compiler_impl::resource_compiler_impl() : cache(*this)
{
  auto add_action = [&](auto& action) {
    actions.insert_or_assign(action.name(), &action);
  };

  // Register all available actions.
  add_action(action_group_resources::singleton_create());
  add_action(action_image_import::singleton_create());
  add_action(action_mesh_export_obj::singleton_create());
  add_action(action_mesh_import_ply::singleton_create());
  add_action(action_mesh_tootle::singleton_create());
  add_action(action_font_import_ttf::singleton_create());
  add_action(action_shader_compile::singleton_create());
  add_action(action_scene_compile::singleton_create());
  add_action(action_scene_import_gltf::singleton_create());
  add_action(action_scene_import_pbrt::singleton_create());
}

resource_compiler_impl::~resource_compiler_impl() = default;

void resource_compiler_impl::read_rules(const fs::path& rules_file_path,
                                        const fs::path& rule_path)
{
  using namespace shift::parser;
  using std::get;
  using std::get_if;

  std::ifstream rules_file;
  rules_file.open(rules_file_path.generic_string(),
                  std::ios::in | std::ios_base::binary);
  if (!rules_file.is_open())
  {
    /// ToDo: log error or throw exception?
    return;
  }
  rules_file.unsetf(std::ios_base::skipws);
  parser::json::value root;
  rules_file >> root;
  rules_file.close();
  json::object& root_object = json::get<json::object>(root);

  auto input_pattern = [&](std::string input) {
    boost::replace_all(input, "<input-path>",
                       regex_escape(input_path.generic_string()));
    boost::replace_all(input, "<build-path>",
                       regex_escape(build_path.generic_string()));
    boost::replace_all(input, "<output-path>",
                       regex_escape(output_path.generic_string()));
    boost::replace_all(input, "<rule-path>",
                       regex_escape(rule_path.generic_string()));
    boost::replace_all(input, "<", "$");
    boost::replace_all(input, ">", "$");
    input = merge_slashes(input);
    try
    {
      return rule_input{input,
                        std::regex{input, std::regex_constants::ECMAScript |
                                            std::regex_constants::icase}};
    }
    catch (const std::regex_error&)
    {
      log::error() << R"(Invalid regular expression ")" << input << '"';
      throw;
    }
  };

  for (const auto& rule_iter : root_object)
  {
    const auto& rule_id = rule_iter.first;
    if (!get_if<json::object>(&rule_iter.second))
      continue;
    const auto& rule_object = get<json::object>(rule_iter.second);

    if (!json::has(rule_object, "pass"))
    {
      log::warning() << rules_file_path.generic_path() << R"(: The rule ")"
                     << rule_id << R"(" lacks the required attribute "pass".)";
      continue;
    }
    const auto& pass_value = rule_object.at("pass");
    if (!get_if<double>(&pass_value))
    {
      log::warning() << rules_file_path.generic_path() << R"(: The rule ")"
                     << rule_id
                     << R"("'s "pass" attribute must be of number type.)";
      continue;
    }

    if (!json::has(rule_object, "action"))
    {
      log::warning() << rules_file_path.generic_path() << R"(: The rule ")"
                     << rule_id
                     << R"(" lacks the required attribute "action".)";
      continue;
    }
    const auto& action_value = rule_object.at("action");
    if (!get_if<std::string>(&action_value))
    {
      log::warning() << rules_file_path.generic_path() << R"(: The rule ")"
                     << rule_id
                     << R"("'s "action" attribute must be of string type.)";
      continue;
    }

    if (!json::has(rule_object, "input"))
    {
      log::warning() << rules_file_path.generic_path() << R"(: The rule ")"
                     << rule_id << R"(" lacks the required attribute "input".)";
      continue;
    }
    const auto& input_value = rule_object.at("input");
    if (!get_if<json::object>(&input_value))
    {
      log::warning() << rules_file_path.generic_path() << R"(: The rule ")"
                     << rule_id
                     << R"("'s "input" attribute must be of object type.)";
      continue;
    }

    if (!json::has(rule_object, "output"))
    {
      log::warning() << R"(The rule ")" << rule_id
                     << R"(" lacks the required attribute "output".)";
      continue;
    }
    const auto& output_value = rule_object.at("output");
    if (!get_if<std::string>(&output_value) &&
        !get_if<json::object>(&output_value))
    {
      log::warning()
        << rules_file_path.generic_path() << R"(: The rule ")" << rule_id
        << R"("'s "output" attribute must be of either string or object type.)";
      continue;
    }

    // Check that the rule's ID is not already in use.
    bool unique_id = true;
    for (const auto& rule : rules)
    {
      if (rule->id == rule_id)
      {
        unique_id = false;
        break;
      }
    }
    if (!unique_id)
    {
      log::error() << R"(Rule id ")" << rule_id
                   << R"(" is not a unique identifier.)";
      continue;
    }

    auto new_rule = std::make_unique<rule_description>();
    new_rule->id = rule_id;
    new_rule->pass =
      boost::lexical_cast<std::uint32_t>(get<double>(pass_value));
    const auto& action_name = get<std::string>(action_value);
    auto action_iter = actions.find(action_name);
    if (action_iter == actions.end())
    {
      log::warning() << rules_file_path.generic_path() << R"(: The rule ")"
                     << rule_id << R"(" references a non-existing action ")"
                     << action_name << R"(".)";
      continue;
    }
    new_rule->action = action_iter->second;
    new_rule->modified = true;
    new_rule->rule_path = rule_path;
    for (const auto& input_object_value : get<json::object>(input_value))
    {
      if (!get_if<std::string>(&input_object_value.second))
      {
        log::warning()
          << rules_file_path.generic_path() << R"(: The rule ")" << rule_id
          << R"("'s "input" attribute object may only contain string values.)";
        continue;
      }
      new_rule->inputs.insert_or_assign(
        input_object_value.first,
        input_pattern(get<std::string>(input_object_value.second)));
    }

    if (json::has(rule_object, "group-by"))
    {
      const auto& group_by_value = rule_object.at("group-by");
      if (!get_if<json::array>(&group_by_value))
      {
        log::warning() << rules_file_path.generic_path() << R"(: The rule ")"
                       << rule_id
                       << R"("'s "group-by" attribute must be of array type.)";
        continue;
      }
      for (const auto& group_by_array_value : get<json::array>(group_by_value))
      {
        if (!get_if<double>(&group_by_array_value))
        {
          log::warning()
            << rules_file_path.generic_path() << R"(: The rule ")" << rule_id
            << R"("'s "group-by" attribute array may only contain number values.)";
          continue;
        }
        new_rule->group_by.insert(
          static_cast<std::size_t>(get<double>(group_by_array_value)));
      }
    }

    if (get_if<std::string>(&output_value))
    {
      new_rule->outputs.insert_or_assign("default",
                                         get<std::string>(output_value));
    }
    else if (get_if<json::object>(&output_value))
    {
      for (const auto& output_object_attribute :
           get<json::object>(output_value))
      {
        if (!get_if<std::string>(&output_object_attribute.second))
        {
          log::warning()
            << rules_file_path.generic_path() << R"(: The rule ")" << rule_id
            << R"("'s "output" attribute object may only contain string attributes.)";
          continue;
        }
        new_rule->outputs.insert_or_assign(
          output_object_attribute.first,
          get<std::string>(output_object_attribute.second));
      }
    }
    else
    {
      BOOST_ASSERT(false);
      return;
    }

    if (!json::has(rule_object, "options"))
    {
      log::warning() << rules_file_path.generic_path() << R"(: The rule ")"
                     << rule_id
                     << R"(" lacks the required attribute "options".)";
      continue;
    }
    new_rule->options = get<json::object>(rule_object.at("options"));

    // Perform a sorted insert into the list of rules which are sorted by pass
    // and rule_path depth.
    auto insert_position = std::upper_bound(
      rules.begin(), rules.end(), new_rule,
      [](const auto& new_rule, const auto& existing_rule) -> bool {
        return new_rule->pass < existing_rule->pass ||
               (new_rule->pass == existing_rule->pass &&
                std::distance(new_rule->rule_path.begin(),
                              new_rule->rule_path.end()) <
                  std::distance(existing_rule->rule_path.begin(),
                                existing_rule->rule_path.end()));
      });
    rules.insert(insert_position, std::move(new_rule));
  }
}

file_description* resource_compiler_impl::save(
  const resource::resource_base& resource, const fs::path& target_name,
  job_description& job)
{
  auto& repository = resource::repository::singleton_instance();
  /// ToDo: We don't want to call resource.id() here! Require id as parameter!
  auto id = resource.id();
  // log::debug() << target_name << ": " << std::hex << std::setw(16)
  //             << std::setfill('0') << id;
  if (!repository.save(resource, resource.type(), id, target_name))
    return nullptr;
  return push(target_name, job);
}

file_description* resource_compiler_impl::push(const fs::path& target_name,
                                               job_description& job)
{
  auto* target = add_file(target_name, job.matching_rule->pass);
  // log::debug() << "Added " << target_name
  //             << " with ts = " << target->last_write_time;
  if (!target)
    return nullptr;
  job.outputs.insert(target);
  return target;
}

file_description* resource_compiler_impl::alias(file_description* source,
                                                std::uint32_t current_pass)
{
  if (!source)
    return nullptr;
  // Fully resolve the requested path.
  auto* result = source;
  while (result->alias && result->alias->pass < current_pass)
    result = result->alias;
  return result;
}

file_description* resource_compiler_impl::add_file(const fs::path& file_path,
                                                   std::uint32_t pass)
{
  std::unique_lock lock(files_mutex);

  boost::system::error_code error_code;
  if (!fs::is_regular_file(file_path, error_code) || error_code)
    return nullptr;

  auto insert_result = files.insert(std::make_pair(file_path, nullptr));
  if (insert_result.second)
    insert_result.first->second = std::make_unique<file_description>(file_path);
  auto* file = insert_result.first->second.get();

  file->last_write_time = fs::last_write_time(file_path, error_code);
  if (error_code)
    return nullptr;
  if (file->pass < pass)
    file->pass = pass;
  file->flags.set(entity_flag::exists).set(entity_flag::modified);

  return file;
}

file_description* resource_compiler_impl::get_file(const fs::path& file_path)
{
  auto file_iter = files.find(file_path);
  if (file_iter != files.end())
    return file_iter->second.get();
  else
    return nullptr;
}

void resource_compiler_impl::match_file(file_description& file,
                                        std::uint32_t current_pass)
{
  file.flags |= entity_flag::used;

  // Skip rule files because they cannot be source of any action.
  if (file.path.filename() == rules_filename)
    return;

  auto match = std::make_unique<input_match>();
  match->file = &file;
  for (auto& rule : rules)
  {
    // Ignore rules of previous passes to avoid infinite recursion.
    if (rule->pass <= current_pass)
      continue;

    // Try to match the relative file path with each rule input regex.
    match->slot_index = 0;
    for (auto input_iter = rule->inputs.begin();
         input_iter != rule->inputs.end(); ++input_iter, ++match->slot_index)
    {
      if (std::regex_search(match->file->generic_string, match->match_results,
                            input_iter->second.pattern))
      {
        match->slot = input_iter;
        std::lock_guard rule_lock(rule->matches_mutex);
        rule->matches.emplace_back(std::move(match));

        if (verbose >= 2)
        {
          log::info() << "File " << file.path << R"( matched against rule ")"
                      << rule->id << '"';
        }
        return;
      }
    }
  }

  if (verbose >= 2)
    log::info() << "File " << file.path << " didn't match against any rule.";
}

std::uint32_t resource_compiler_impl::next_pass(
  std::uint32_t previous_pass,
  std::shared_lock<std::shared_mutex>& /* rules_read_lock */)
{
  std::uint32_t next_pass = 0;
  for (auto& rule : rules)
  {
    if (!rule->matches.empty() && rule->pass > previous_pass &&
        (next_pass == 0 || next_pass > rule->pass))
    {
      next_pass = rule->pass;
    }
  }
  return next_pass;
}

std::vector<job_description*> resource_compiler_impl::query_jobs(
  std::uint32_t pass, std::shared_lock<std::shared_mutex>& /* read_lock */)
{
  std::vector<job_description*> job_pointers;

  // Process each rule's matches separately.
  for (auto& rule : rules)
  {
    // Skip rules that are not in the current pass.
    if (rule->pass != pass)
      continue;

    std::vector<std::unique_ptr<job_description>> new_jobs;
    for (auto& new_match : rule->matches)
    {
      // Multiple matches may be grouped together into a single job when the
      // rule's group_by set is not empty.
      bool merged_with_existing_job = false;
      if (!rule->group_by.empty())
      {
        // Check whether this match shall be added to an existing job or a new
        // one.
        for (auto& existing_job : new_jobs)
        {
          // We only need to compare the new match with the first match
          // already in this job.
          auto& existing_match = existing_job->inputs.at(0);
          if (existing_match->match_results.size() !=
              new_match->match_results.size())
          {
            // This should basically never happen.
            BOOST_ASSERT(false);
            continue;
          }

          bool do_match = true;
          for (std::size_t match_id = 1;
               match_id < new_match->match_results.size(); ++match_id)
          {
            if (rule->group_by.find(match_id) != rule->group_by.end())
              continue;
            if (existing_match->match_results[match_id].str() !=
                new_match->match_results[match_id].str())
            {
              do_match = false;
              break;
            }
          }
          if (do_match)
          {
            existing_job->inputs.emplace_back(std::move(new_match));
            merged_with_existing_job = true;
            break;
          }
        }
      }
      if (!merged_with_existing_job)
      {
        if (verbose >= 2)
          log::debug() << R"(Adding new job based on rule ")" << rule->id
                       << '"';
        auto new_job = std::make_unique<job_description>();
        new_job->matching_rule = rule.get();
        new_job->inputs.emplace_back(std::move(new_match));
        new_jobs.push_back(std::move(new_job));
      }
    }

    // All matches of this rule should have been moved to jobs at this point.
    rule->matches.clear();

    // Move newly created jobs to the global list.
    for (auto& new_job : new_jobs)
      jobs.emplace_back(std::move(new_job));
  }

  // Add all modified jobs to the list of jobs to process.
  for (auto& job : jobs)
  {
    if (job->matching_rule->pass == pass)
    {
      bool skip_job = true;
      // Check if there is an equivalent job in our cache.
      if (const auto* cached_job = cache.get_job(*job))
      {
        // If so, copy the cached job output files over.
        for (const auto* output_file : cached_job->outputs)
        {
          if (auto* file = add_file(output_file->path, pass))
            job->outputs.insert(file);
          else
          {
            // If one of the cached output files does not exist we have to
            // process this job.
            // log::debug() << "Won't skip job because output file "
            //             << output_file->path << " cannot be found.";
            job->outputs.clear();
            skip_job = false;
            break;
          }
        }

        if (skip_job && !cache.is_modified(*job))
        {
          // If the job is indeed unmodified we can eventually adopt alias links
          // from input to output files.
          for (const auto& cached_input_match : cached_job->inputs)
          {
            if (cached_input_match->file->alias)
            {
              for (const auto& input_match : job->inputs)
              {
                if (input_match->file->hash == cached_input_match->file->hash &&
                    input_match->file->path == cached_input_match->file->path)
                {
                  for (auto* output_file : job->outputs)
                  {
                    if (output_file->hash ==
                          cached_input_match->file->alias->hash &&
                        output_file->path ==
                          cached_input_match->file->alias->path)
                    {
                      input_match->file->alias = output_file;
                      break;
                    }
                  }
                  break;
                }
              }
            }
          }
        }
        else
        {
          // The job is modified because one of the dependencies (action, rule,
          // input, or output) is modified.
          // log::debug() << "Won't skip job because some dependency is
          // modified.";
          skip_job = false;
        }
      }
      else
      {
        // There is no cached job.
        // log::debug() << "Won't skip job because there is no cached
        // equivalent.";
        skip_job = false;
      }

      if (!skip_job)
        job->flags.set(entity_flag::modified);
      job_pointers.push_back(job.get());
    }
  }

  return job_pointers;
}

// bool path_contains_file(path dir, path file)
//{
//  // If dir ends with "/" and isn't the root directory, then the final
//  // component returned by iterators will include "." and will interfere
//  // with the std::equal check below, so we strip it before proceeding.
//  if (dir.filename() == ".")
//    dir.remove_filename();
//  // We're also not interested in the file's name.
//  BOOST_ASSERT(file.has_filename());
//  file.remove_filename();

//  // If dir has more components than file, then file can't possibly
//  // reside in dir.
//  auto dir_len = std::distance(dir.begin(), dir.end());
//  auto file_len = std::distance(file.begin(), file.end());
//  if (dir_len > file_len)
//    return false;

//  // This stops checking when it reaches dir.end(), so it's OK if file
//  // has more directory components afterward. They won't be checked.
//  return std::equal(dir.begin(), dir.end(), file.begin());
//}
}
