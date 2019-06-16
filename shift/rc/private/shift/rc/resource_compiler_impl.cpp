#include "shift/rc/resource_compiler_impl.hpp"
#include "shift/rc/types.hpp"
#include "shift/rc/action_group_resources.hpp"
#include "shift/rc/action_image_import_tiff.hpp"
#include "shift/rc/action_image_export_tiff.hpp"
#include "shift/rc/action_mesh_tootle.hpp"
#include "shift/rc/action_mesh_import_ply.hpp"
#include "shift/rc/action_mesh_export_obj.hpp"
#include "shift/rc/action_font_import_ttf.hpp"
#include "shift/rc/action_shader_compile.hpp"
#include "shift/rc/action_scene_compile.hpp"
#include "shift/rc/action_scene_import_gltf.hpp"
#include "shift/rc/action_scene_import_pbrt.hpp"
#include <shift/resource_db/repository.hpp>
#include <shift/parser/json/json.hpp>
#include <shift/log/log.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <chrono>
#include <map>
#include <set>
#include <fstream>
#include <iostream>
#include <filesystem>

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
    cache.register_action(action.action_name, action.action_version, action);
  };

  // Register all available actions.
  add_action(action_group_resources::singleton_create());
  add_action(action_image_import_tiff::singleton_create());
  add_action(action_image_export_tiff::singleton_create());
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
  using namespace std::string_literals;
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
    boost::replace_all(input, "<input-path/>",
                       regex_escape(input_path.generic_string() + '/'));
    boost::replace_all(input, "<build-path/>",
                       regex_escape(build_path.generic_string() + '/'));
    boost::replace_all(input, "<output-path/>",
                       regex_escape(output_path.generic_string() + '/'));
    boost::replace_all(
      input, "<rule-path/>",
      rule_path.empty() ? ""s : regex_escape(rule_path.generic_string() + '/'));
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
      log::warning() << rules_file_path << R"(: The rule ")" << rule_id
                     << R"(" lacks the required attribute "pass".)";
      continue;
    }
    const auto& pass_value = rule_object.at("pass");
    if (!get_if<std::int64_t>(&pass_value))
    {
      log::warning() << rules_file_path << R"(: The rule ")" << rule_id
                     << R"("'s "pass" attribute must be of number type.)";
      continue;
    }

    if (!json::has(rule_object, "action"))
    {
      log::warning() << rules_file_path << R"(: The rule ")" << rule_id
                     << R"(" lacks the required attribute "action".)";
      continue;
    }
    const auto& action_value = rule_object.at("action");
    if (!get_if<std::string>(&action_value))
    {
      log::warning() << rules_file_path << R"(: The rule ")" << rule_id
                     << R"("'s "action" attribute must be of string type.)";
      continue;
    }

    if (!json::has(rule_object, "input"))
    {
      log::warning() << rules_file_path << R"(: The rule ")" << rule_id
                     << R"(" lacks the required attribute "input".)";
      continue;
    }
    const auto& input_value = rule_object.at("input");
    if (!get_if<json::object>(&input_value))
    {
      log::warning() << rules_file_path << R"(: The rule ")" << rule_id
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
        << rules_file_path << R"(: The rule ")" << rule_id
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

    rule_create_info rule_info;
    rule_info.id = rule_id;
    rule_info.pass =
      boost::lexical_cast<std::uint32_t>(get<std::int64_t>(pass_value));
    const auto& action_name = get<std::string>(action_value);
    rule_info.action = cache.find_action(action_name);
    if (rule_info.action == nullptr)
    {
      log::warning() << rules_file_path << R"(: The rule ")" << rule_id
                     << R"(" references a non-existing action ")" << action_name
                     << R"(".)";
      continue;
    }
    rule_info.path = rule_path;
    for (const auto& input_object_value : get<json::object>(input_value))
    {
      if (!get_if<std::string>(&input_object_value.second))
      {
        log::warning()
          << rules_file_path << R"(: The rule ")" << rule_id
          << R"("'s "input" attribute object may only contain string values.)";
        continue;
      }
      rule_info.inputs.insert_or_assign(
        input_object_value.first,
        input_pattern(get<std::string>(input_object_value.second)));
    }

    if (json::has(rule_object, "group-by"))
    {
      const auto& group_by_value = rule_object.at("group-by");
      if (!get_if<json::array>(&group_by_value))
      {
        log::warning() << rules_file_path << R"(: The rule ")" << rule_id
                       << R"("'s "group-by" attribute must be of array type.)";
        continue;
      }
      for (const auto& group_by_array_value : get<json::array>(group_by_value))
      {
        if (!get_if<std::int64_t>(&group_by_array_value))
        {
          log::warning()
            << rules_file_path << R"(: The rule ")" << rule_id
            << R"("'s "group-by" attribute array may only contain number values.)";
          continue;
        }
        rule_info.group_by.insert(
          static_cast<std::size_t>(get<std::int64_t>(group_by_array_value)));
      }
    }

    if (get_if<std::string>(&output_value))
    {
      rule_info.outputs.insert_or_assign("default",
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
            << rules_file_path << R"(: The rule ")" << rule_id
            << R"("'s "output" attribute object may only contain string attributes.)";
          continue;
        }
        rule_info.outputs.insert_or_assign(
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
      log::warning() << rules_file_path << R"(: The rule ")" << rule_id
                     << R"(" lacks the required attribute "options".)";
      continue;
    }
    rule_info.options = get<json::object>(rule_object.at("options"));

    auto& new_rule = cache.get_rule(rule_info);

    // Perform a sorted insert into the list of rules which are sorted by pass
    // and rule_path depth.
    auto insert_position = std::upper_bound(
      rules.begin(), rules.end(), new_rule,
      [](const auto& new_rule, const auto& existing_rule) -> bool {
        return new_rule.pass < existing_rule->pass ||
               (new_rule.pass == existing_rule->pass &&
                std::distance(new_rule.path.begin(), new_rule.path.end()) <
                  std::distance(existing_rule->path.begin(),
                                existing_rule->path.end()));
      });
    rules.insert(insert_position, &new_rule);
  }
}

file_description* resource_compiler_impl::save(
  const resource_db::resource_base& resource, const fs::path& target_name,
  job_description& job)
{
  auto& repository = resource_db::repository::singleton_instance();
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
  auto* target = add_file(target_name, job.rule->pass);
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

  std::error_code error_code;
  if (!fs::is_regular_file(file_path, error_code) || error_code)
    return nullptr;

  auto insert_result = files.insert(std::make_pair(file_path, nullptr));
  if (insert_result.second)
    insert_result.first->second = std::make_unique<file_description>(file_path);
  auto* file = insert_result.first->second.get();

#if defined(_MSC_VER)
  // This ugly hack attempts to convert a std::filesystem::file_time_type to
  // std::chrono::system_clock::time_point. It likely produces wrong results
  // because there is no guarantee that both clocks have the same epoch.
  // C++20 will fix this issue and make clocks convertible.
  auto last_write_time = fs::last_write_time(file_path, error_code);
  if (error_code)
    return nullptr;
  file->last_write_time =
    std::chrono::system_clock::time_point(std::chrono::system_clock::duration(
      decltype(last_write_time)::clock::duration::rep{
        last_write_time.time_since_epoch().count()}));
#else
  file->last_write_time = fs::last_write_time(file_path, error_code);
  if (error_code)
    return nullptr;
#endif
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

std::vector<std::unique_ptr<job_description>>
resource_compiler_impl::query_jobs(
  std::uint32_t pass, std::shared_lock<std::shared_mutex>& /* read_lock */)
{
  std::vector<std::unique_ptr<job_description>> jobs;

  // Process each rule's matches separately.
  for (auto* rule : rules)
  {
    // Skip rules that are not in the current pass.
    if (rule->pass != pass)
      continue;

    for (auto& new_match : rule->matches)
    {
      // Multiple matches may be grouped together into a single job when the
      // rule's group_by set is not empty.
      bool merged_with_existing_job = false;
      if (!rule->group_by.empty())
      {
        // Check whether this match shall be merged into an existing job of the
        // same rule.
        for (auto existing_job_iter = jobs.rbegin();
             existing_job_iter != jobs.rend(); ++existing_job_iter)
        {
          auto& existing_job = **existing_job_iter;
          if (existing_job.rule != rule)
            break;

          bool do_match = true;
          if (const auto& existing_match_iter =
                existing_job.inputs.find(new_match->slot_index);
              existing_match_iter != existing_job.inputs.end())
          {
            // We only need to compare the new match with the first match
            // already in this job.
            const auto& existing_match = *existing_match_iter->second;
            if (existing_match.match_results.size() !=
                new_match->match_results.size())
            {
              // This should never happen.
              BOOST_ASSERT(false);
              continue;
            }
            for (std::size_t match_id = 1;
                 match_id < new_match->match_results.size(); ++match_id)
            {
              if (rule->group_by.find(match_id) != rule->group_by.end())
                continue;
              if (existing_match.match_results[match_id].str() !=
                  new_match->match_results[match_id].str())
              {
                do_match = false;
                break;
              }
            }
          }

          if (do_match)
          {
            auto slot_index = new_match->slot_index;
            existing_job.inputs.insert({slot_index, std::move(new_match)});
            merged_with_existing_job = true;
            break;
          }
        }
      }
      if (!merged_with_existing_job)
      {
        if (verbose >= 2)
        {
          log::debug() << R"(Adding new job based on rule ")" << rule->id
                       << '"';
        }
        auto& new_job = jobs.emplace_back(std::make_unique<job_description>());
        new_job->rule = rule;
        auto slot_index = new_match->slot_index;
        new_job->inputs.insert({slot_index, std::move(new_match)});
      }
    }

    // All matches of this rule should have been moved to jobs at this point.
    rule->matches.clear();
  }

  // Add all modified jobs to the list of jobs to process.
  for (auto& job : jobs)
  {
    if (job->rule->pass == pass)
    {
      bool skip_job = true;
      // Check if there is an equivalent job in our cache.
      if (const auto* cached_job = cache.get_job(*job);
          cached_job != nullptr && !cache.is_modified(*cached_job))
      {
        // If so, copy the cached job output files over.
        for (const auto* cached_output : cached_job->outputs)
        {
          if (auto* output_file = add_file(cached_output->path, pass);
              output_file != nullptr)
            job->outputs.insert(output_file);
          else
          {
            // If one of the cached output files does not exist we have to
            // process this job.
            // log::debug() << "Won't skip job because output file "
            //             << cached_output->path << " cannot be found.";
            job->outputs.clear();
            skip_job = false;
            break;
          }
        }

        if (skip_job && !cache.is_modified(*job))
        {
          // If the job is indeed unmodified we can eventually adopt alias links
          // from input to output files.
          for (const auto& [cached_input_slot_index, cached_input_match] :
               cached_job->inputs)
          {
            if (cached_input_match->file->alias)
            {
              for (const auto& [input_slot_index, input_match] : job->inputs)
              {
                if (cached_input_slot_index == input_slot_index &&
                    input_match->file->hash == cached_input_match->file->hash &&
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
        /// ToDo: Do this later!
        // cache.add_job(*job);
      }

      if (!skip_job)
        job->flags.set(entity_flag::modified);
    }
  }

  return jobs;
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
