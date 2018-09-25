#include "shift/rc/types.h"
#include "shift/rc/resource_compiler.h"
#include <boost/algorithm/string/replace.hpp>

namespace shift::rc
{
bool operator==(const rule_description& lhs, const rule_description& rhs)
{
  if (lhs.id != rhs.id)
    return false;
  if (lhs.pass != rhs.pass)
    return false;
  if (lhs.action->name() != rhs.action->name())
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
  if (lhs.matching_rule != rhs.matching_rule)
    return false;

  if (lhs.inputs.size() != rhs.inputs.size())
    return false;
  for (auto left = lhs.inputs.begin(); left != lhs.inputs.end(); ++left)
  {
    bool found = false;
    for (auto right = rhs.inputs.begin(); right != rhs.inputs.end(); ++right)
    {
      if ((*left)->file->hash == (*right)->file->hash &&
          (*left)->file->path == (*right)->file->path)
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
  return matching_rule->outputs.find(name) != matching_rule->outputs.end();
}

fs::path job_description::output(
  const std::string& name,
  const std::map<std::string, std::string>& custom_variables) const
{
  BOOST_ASSERT(!inputs.empty());
  if (inputs.empty())
    return "";

  auto& compiler = resource_compiler::singleton_instance();

  auto output_iter = matching_rule->outputs.find(name);
  if (output_iter == matching_rule->outputs.end())
  {
    BOOST_ASSERT(false);
    return compiler.build_path() / name / "no-output.error";
  }
  auto path_string = output_iter->second;

  boost::replace_all(path_string, "<input-path>",
                     compiler.input_path().generic_string());
  boost::replace_all(path_string, "<build-path>",
                     compiler.build_path().generic_string());
  boost::replace_all(path_string, "<output-path>",
                     compiler.output_path().generic_string());
  boost::replace_all(path_string, "<rule-path>",
                     matching_rule->rule_path.generic_string());

  for (const auto& input : inputs)
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

std::string merge_slashes(const std::string& input)
{
  static const std::regex successive_slashes("//+", std::regex::ECMAScript);
  return std::regex_replace(input, successive_slashes, "/");
}
}