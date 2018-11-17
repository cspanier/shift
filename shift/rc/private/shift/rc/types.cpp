#include "shift/rc/types.hpp"
#include "shift/rc/resource_compiler.hpp"
#include <boost/algorithm/string/replace.hpp>

namespace shift::rc
{
rule_description::rule_description(const rule_create_info& create_info)
: rule_create_info(create_info)
{
}

bool operator==(const rule_description& lhs, const rule_description& rhs)
{
  if (lhs.id != rhs.id)
    return false;
  if (lhs.pass != rhs.pass)
    return false;
  if (lhs.action->impl != rhs.action->impl)
    return false;

  if (lhs.inputs.size() != rhs.inputs.size())
    return false;
  for (const auto& lhs_input : lhs.inputs)
  {
    auto rhs_input_iter = rhs.inputs.find(lhs_input.first);
    if (rhs_input_iter == rhs.inputs.end())
      return false;
    if (lhs_input.second.source != rhs_input_iter->second.source)
      return false;
  }

  if (lhs.group_by.size() != rhs.group_by.size())
    return false;
  for (const auto lhs_group_by : lhs.group_by)
  {
    if (rhs.group_by.find(lhs_group_by) == rhs.group_by.end())
      return false;
  }

  if (lhs.outputs.size() != rhs.outputs.size())
    return false;
  for (const auto& lhs_output : lhs.outputs)
  {
    auto rhs_output_iter = rhs.outputs.find(lhs_output.first);
    if (rhs_output_iter == rhs.outputs.end())
      return false;
    if (lhs_output.second != rhs_output_iter->second)
      return false;
  }

  if (lhs.options.size() != rhs.options.size())
    return false;
  for (const auto& lhs_option : lhs.options)
  {
    auto rhs_option_iter = rhs.options.find(lhs_option.first);
    if (rhs_option_iter == rhs.options.end())
      return false;
    if (lhs_option.second != rhs_option_iter->second)
      return false;
  }

  return true;
}

bool operator==(const job_description& lhs, const job_description& rhs)
{
  if (lhs.rule != rhs.rule)
    return false;

  if (lhs.inputs.size() != rhs.inputs.size())
    return false;
  for (auto left_iter = lhs.inputs.begin(); left_iter != lhs.inputs.end();
       ++left_iter)
  {
    bool found = false;
    for (auto right_iter = rhs.inputs.begin(); right_iter != rhs.inputs.end();
         ++right_iter)
    {
      auto& [left_slot_index, left_match] = *left_iter;
      auto& [right_slot_index, right_match] = *right_iter;
      if (left_slot_index == right_slot_index &&
          left_match->file->hash == right_match->file->hash &&
          left_match->file->path == right_match->file->path)
      {
        found = true;
        break;
      }
    }
    if (!found)
      return false;
  }
  return true;
}

bool job_description::has_output(const std::string& name) const
{
  return rule->outputs.find(name) != rule->outputs.end();
}

fs::path job_description::output_file_path(
  const std::string& name,
  const std::map<std::string, std::string>& custom_variables) const
{
  using namespace std::string_literals;

  BOOST_ASSERT(!inputs.empty());
  if (inputs.empty())
    return "";

  auto& compiler = resource_compiler::singleton_instance();

  auto output_iter = rule->outputs.find(name);
  if (output_iter == rule->outputs.end())
  {
    BOOST_ASSERT(false);
    return compiler.build_path() / name / "no-output.error";
  }
  auto path_string = output_iter->second;

  boost::replace_all(path_string, "<input-path/>",
                     compiler.input_path().generic_string() + '/');
  boost::replace_all(path_string, "<build-path/>",
                     compiler.build_path().generic_string() + '/');
  boost::replace_all(path_string, "<output-path/>",
                     compiler.output_path().generic_string() + '/');
  boost::replace_all(
    path_string, "<rule-path/>",
    rule->path.empty() ? ""s : rule->path.generic_string() + '/');

  for (const auto& [input_slot_index, input] : inputs)
  {
    std::size_t match_index = 0;
    for (const auto& match_result : input->match_results)
    {
      if (match_result.matched)
      {
        std::stringstream variable_name;
        variable_name << "<" << input->slot->first << ":"
                      << std::to_string(match_index) << ">";
        boost::replace_all(path_string, variable_name.str(),
                           match_result.str());
      }
      ++match_index;
    }
  }
  for (const auto& custom_variable : custom_variables)
  {
    boost::replace_all(path_string, "<" + custom_variable.first + ">",
                       custom_variable.second);
  }
  boost::replace_all(path_string, "<", "$");
  boost::replace_all(path_string, ">", "$");
  path_string = merge_slashes(path_string);
  return path_string;
}

void job_description::mark_as_used()
{
  flags.set(entity_flag::used);
  for (auto& [input_slot_index, input] : inputs)
    input->file->flags.set(entity_flag::used);
  for (auto* output : outputs)
    output->flags.set(entity_flag::used);
}

action_base::action_base(const std::string& action_name,
                         const action_version& action_version,
                         const bool action_support_multithreading)
: _description({action_name, action_version, this}),
  _support_multithreading(action_support_multithreading)
{
}

action_base::~action_base() = default;

const action_description& action_base::description() const
{
  return _description;
}

bool action_base::support_multithreading() const
{
  return _support_multithreading;
}

std::string merge_slashes(const std::string& input)
{
  static const std::regex successive_slashes("//+", std::regex::ECMAScript);
  return std::regex_replace(input, successive_slashes, "/");
}
}
