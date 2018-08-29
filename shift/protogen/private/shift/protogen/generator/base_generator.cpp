#include "shift/protogen/generator/base_generator.h"
#include "shift/core/string_util.h"
#include <sstream>
#include <vector>
#include <functional>
#include <shift/core/boost_disable_warnings.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/path_traits.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/program_options/options_description.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <shift/core/string_util.h>

namespace shift::proto::generator
{
base_generator::~base_generator() = default;

type_path base_generator::relative_path(
  const base_generator::name_lookup& name_lookup, const node* node,
  const namescope* current_scope)
{
  if (!node)
    return type_path();
  auto name = name_lookup(*node);
  if (current_scope)
  {
    while (node->depth < current_scope->depth)
      current_scope = current_scope->parent;
    if (node->depth == current_scope->depth &&
        name_lookup(*current_scope) == name)
    {
      return type_path();
    }
  }
  auto result = relative_path(name_lookup, node->parent, current_scope);
  if (!name.empty())
    result.push_back(name);
  return result;
}

std::pair<const namescope&, const type_reference&>
base_generator::make_scope_and_type(const namescope& namescope,
                                    const type_reference& type_reference)
{
  return std::make_pair(std::ref(namescope), std::ref(type_reference));
}

std::string base_generator::delimiter_case(std::string name)
{
  std::stringstream result;
  bool last_upper = false;
  for (auto& c : name)
  {
    if (core::is_upper(c))
    {
      if (!last_upper && result.tellp() != std::streampos{0})
        result << '_';
      result << core::to_lower(c);
      last_upper = true;
    }
    else
    {
      result << c;
      last_upper = false;
    }
  }
  return result.str();
}

std::string base_generator::pascal_case(std::string name)
{
  if (!name.empty())
    name[0] = core::to_upper(name[0]);
  return name;
}

std::string base_generator::camel_case(std::string name)
{
  for (auto& c : name)
  {
    if (core::is_upper(c))
      c = core::to_lower(c);
    else
      break;
  }

  std::stringstream result;
  for (auto& c : name)
  {
    if (c != '_')
      result << c;
  }
  return result.str();
}

std::string base_generator::upper_case(std::string name)
{
  std::stringstream result;
  for (auto& c : name)
  {
    if (c != '_')
      result << core::to_upper(c);
  }
  return result.str();
}

std::string base_generator::lower_case(std::string name)
{
  std::stringstream result;
  for (auto& c : name)
  {
    if (c != '_')
      result << core::to_lower(c);
  }
  return result.str();
}
}
