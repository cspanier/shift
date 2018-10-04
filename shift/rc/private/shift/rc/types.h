#ifndef SHIFT_RC_TYPES_H
#define SHIFT_RC_TYPES_H

#include <functional>
#include <string>
#include <set>
#include <unordered_set>
#include <map>
#include <chrono>
#include <regex>
#include <mutex>
#include <boost/functional/hash/hash.hpp>
#include <boost/filesystem/path.hpp>
#include <shift/core/types.h>
#include <shift/core/bit_field.h>
#include <shift/parser/json/json.h>
#include <shift/parser/json/hash.h>
#include "shift/rc/action_base.h"

namespace shift::rc
{
namespace fs = boost::filesystem;

class resource_compiler;
class resource_compiler_impl;
struct rule_description;
struct input_match;
struct job_description;
struct file_stats;
struct cache_data;

///
enum class entity_flag
{
  exists = 0b0001,
  modified = 0b0010,
  used = 0b0100,
  failed = 0b1000
};

using entity_flags = core::bit_field<entity_flag>;

/// Stores a filename regex and a copy of the pattern string from which the
/// regex was constructed from.
struct rule_input
{
  std::string source;
  std::regex pattern;
};

/// A rule defines what input files are processed by which action into what
/// output files.
struct rule_description
{
  rule_description() = default;
  rule_description(const rule_description&) = delete;
  rule_description(rule_description&&) = delete;
  rule_description& operator=(const rule_description&) = delete;
  rule_description& operator=(rule_description&&) = delete;

  std::string id;
  std::uint32_t pass;
  const action_base* action = nullptr;
  fs::path rule_path;
  std::map<std::string, rule_input> inputs;
  std::set<std::size_t> group_by;
  std::map<std::string, std::string> outputs;
  parser::json::object options;
  bool modified;

  std::mutex matches_mutex;
  std::vector<std::unique_ptr<input_match>> matches;
};

/// Comparison operator used for finding rules in cache.
bool operator==(const rule_description& lhs, const rule_description& rhs);

/// Input matches are generated from the list of source files against all
/// defined rules.
/// @remarks
///   std::smatch stores iterators into the string that matched the regex. We
///   can neither move nor swap the string to a different place without
///   invalidating those iterators (and thus the smatch object) and need to
///   dynamically allocate instances of input_match to work around this
///   limitation. To prevent any errors this type is non-copyable and
///   non-movable.
struct input_match
{
  input_match() = default;
  input_match(const input_match&) = delete;
  input_match(input_match&&) = delete;
  input_match& operator=(const input_match&) = delete;
  input_match& operator=(input_match&&) = delete;

  std::size_t slot_index = 0;

  /// An iterator pointing to the matching rule_description::inputs map entry.
  std::map<std::string, rule_input>::const_iterator slot;

  file_stats* file = nullptr;

  /// The regex search match pointing into generic_string.
  std::smatch match_results;
};

/// A job description contains all information needed to feed any action.
/// @remarks
///   Jobs are generated from the sets of input matches on actions. This
///   additional indirection is needed to group multiple input matches together
///   to a single job.
struct job_description
{
  bool has_output(const std::string& name) const;

  fs::path output(
    const std::string& name,
    const std::map<std::string, std::string>& custom_variables) const;

  rule_description* matching_rule = nullptr;
  std::vector<std::unique_ptr<input_match>> inputs;
  std::unordered_set<file_stats*> outputs;
  entity_flags flags = entity_flags{0};
};

/// Comparison operator used for finding jobs in cache.
bool operator==(const job_description& lhs, const job_description& rhs);

///
struct file_stats
{
  /// Constructor.
  file_stats(const fs::path& path)
  : path(path),
    generic_string(path.generic_string()),
    hash(std::hash<std::string>{}(generic_string))
  {
  }

  file_stats(const file_stats&) = delete;
  file_stats(file_stats&&) = delete;
  ~file_stats() = default;
  file_stats& operator=(const file_stats&) = delete;
  file_stats& operator=(file_stats&&) = delete;

  fs::path path;
  /// Stores the result of path.generic_string(). This string is required as we
  /// match it against rule regexes.
  std::string generic_string;
  std::size_t hash;
  time_t last_write_time = 0;
  std::uint32_t pass = 0;
  entity_flags flags = entity_flags{0};
  file_stats* alias = nullptr;
};

/// In some circumstances variables are replaced with empty strings (e.g.
/// "<build-path>/<rule-path>/bar" might get to "foo//bar"). In this case
/// multiple slashes need to be merged.
std::string merge_slashes(const std::string& input);
}

namespace std
{
template <>
struct hash<boost::filesystem::path>
{
  std::size_t operator()(const boost::filesystem::path& path) const
  {
    return std::hash<std::string>{}(path.generic_string());
  }
};

template <>
struct hash<shift::rc::rule_description>
{
  std::size_t operator()(const shift::rc::rule_description& rule) const
  {
    std::size_t seed = std::hash<std::string>{}(rule.id);
    boost::hash_combine(seed, std::hash<std::uint32_t>{}(rule.pass));
    boost::hash_combine(seed, std::hash<std::string>{}(rule.action->name()));

    boost::hash_combine(seed, std::hash<std::size_t>{}(rule.inputs.size()));
    for (const auto& input : rule.inputs)
    {
      boost::hash_combine(seed, std::hash<std::string>{}(input.first));
      boost::hash_combine(seed, std::hash<std::string>{}(input.second.source));
    }

    boost::hash_combine(seed, std::hash<std::size_t>{}(rule.group_by.size()));
    for (const auto index : rule.group_by)
      boost::hash_combine(seed, std::hash<std::size_t>{}(index));

    boost::hash_combine(seed, std::hash<std::size_t>{}(rule.outputs.size()));
    for (const auto& output : rule.outputs)
    {
      boost::hash_combine(seed, std::hash<std::string>{}(output.first));
      boost::hash_combine(seed, std::hash<std::string>{}(output.second));
    }

    boost::hash_combine(seed, std::hash<std::size_t>{}(rule.options.size()));
    for (const auto& option : rule.options)
    {
      boost::hash_combine(seed, std::hash<std::string>{}(option.first));
      boost::hash_combine(
        seed, std::hash<shift::parser::json::value>{}(option.second));
    }

    return seed;
  }
};

template <>
struct hash<shift::rc::job_description>
{
  std::size_t operator()(const shift::rc::job_description& job) const
  {
    std::size_t seed = std::hash<std::string>{}(job.matching_rule->id);
    boost::hash_combine(seed, std::hash<std::size_t>{}(job.inputs.size()));
    for (const auto& input : job.inputs)
    {
      boost::hash_combine(
        seed, std::hash<std::string>{}(input->file->generic_string));
    }
    return seed;
  }
};
}

#endif
