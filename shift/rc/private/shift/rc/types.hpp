#ifndef SHIFT_RC_TYPES_HPP
#define SHIFT_RC_TYPES_HPP

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
#include <shift/core/types.hpp>
#include <shift/core/bit_field.hpp>
#include <shift/parser/json/json.hpp>
#include <shift/parser/json/hash.hpp>

namespace shift::rc
{
namespace fs = boost::filesystem;

class resource_compiler;
class resource_compiler_impl;
struct rule_description;
struct input_match;
struct job_description;
struct file_description;
struct cache_data;

///
enum class entity_flag : std::uint32_t
{
  exists = (1u << 0),
  modified = (1u << 1),
  used = (1u << 2),
  failed = (1u << 3)
};

using entity_flags = core::bit_field<entity_flag>;

using action_version = std::string;
class action_base;

/// A short description to uniquely identify each action.
struct action_description
{
  action_description() = default;

  action_description(std::string name, action_version version,
                     const action_base* impl,
                     entity_flags flags = entity_flags{})
  : name(name), version(version), impl(impl), flags(flags)
  {
  }

  action_description(const action_description&) = default;
  action_description(action_description&&) = default;
  ~action_description() = default;
  action_description& operator=(const action_description&) = default;
  action_description& operator=(action_description&&) = default;

  const std::string name;
  const action_version version;
  const action_base* impl = nullptr;
  entity_flags flags;
};

/// Stores a filename regex and a copy of the pattern string from which the
/// regex was constructed from.
struct rule_input
{
  std::string source;
  std::regex pattern;
};

struct rule_create_info
{
  std::string id;
  std::uint32_t pass = {};
  const action_description* action = nullptr;
  fs::path path;
  std::map<std::string, rule_input> inputs;
  std::set<std::size_t> group_by;
  std::map<std::string, std::string> outputs;
  parser::json::object options;
  entity_flags flags = {};
};

/// A rule defines what input files are processed by which action into what
/// output files.
struct rule_description : public rule_create_info
{
  rule_description() = default;
  rule_description(const rule_create_info& create_info);
  rule_description(const rule_description&) = delete;
  rule_description(rule_description&&) = delete;
  rule_description& operator=(const rule_description&) = delete;
  rule_description& operator=(rule_description&&) = delete;

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

  file_description* file = nullptr;

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

  /// Marks this job and all input and output files as used.
  void mark_as_used();

  std::size_t id = 0;
  rule_description* rule = nullptr;
  std::unordered_multimap<std::size_t, std::unique_ptr<input_match>> inputs;
  std::unordered_set<file_description*> outputs;
  entity_flags flags = entity_flags{0};
};

/// Comparison operator used for finding jobs in cache.
bool operator==(const job_description& lhs, const job_description& rhs);

/// Base class for all actions.
/// @remarks
///   Each resource processing algorithm is encapsulated in an action.
class action_base
{
public:
  /// Constructor.
  /// @param action_support_multithreading
  ///   Set this parameter to false to enforce all instances of this action to
  ///   be processed in a serial fashion. This is required e.g. when using
  ///   external libs that make use of global/static variables to store state.
  action_base(const std::string& action_name,
              const action_version& action_version,
              bool action_support_multithreading = true);

  action_base(const action_base&) = delete;
  action_base(action_base&&) = delete;

  /// Destructor.
  virtual ~action_base() = 0;

  action_base& operator=(const action_base&) = delete;
  action_base& operator=(action_base&&) = delete;

  /// Returns the action's description.
  const action_description& description() const;

  /// Returns whether the action can be used from multiple threads.
  bool support_multithreading() const;

  /// Process a single job.
  /// @remarks
  ///   This method may be called from multiple threads in parallel. You are
  ///   supposed to store most runtime data on the stack to avoid
  ///   synchronization issues.
  virtual bool process(resource_compiler_impl& compiler,
                       job_description& job) const = 0;

private:
  action_description _description;
  bool _support_multithreading = true;
};

///
struct file_description
{
  /// Constructor.
  file_description(const fs::path& path)
  : path(path),
    generic_string(path.generic_string()),
    hash(std::hash<std::string>{}(generic_string))
  {
  }

  file_description(const file_description&) = delete;
  file_description(file_description&&) = delete;
  ~file_description() = default;
  file_description& operator=(const file_description&) = delete;
  file_description& operator=(file_description&&) = delete;

  fs::path path;
  /// Stores the result of path.generic_string(). This string is required as we
  /// match it against rule regexes.
  std::string generic_string;
  std::size_t hash;
  time_t last_write_time = 0;
  std::uint32_t pass = 0;
  entity_flags flags = entity_flags{0};
  file_description* alias = nullptr;
  /// Each file may have zero or one source.
  job_description* source = nullptr;
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
    boost::hash_combine(seed, std::hash<std::string>{}(rule.action->name));

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
    std::size_t seed = std::hash<std::string>{}(job.rule->id);
    boost::hash_combine(seed, std::hash<std::size_t>{}(job.inputs.size()));
    for (const auto& [slot_index, input] : job.inputs)
    {
      boost::hash_combine(seed, slot_index);
      boost::hash_combine(
        seed, std::hash<std::string>{}(input->file->generic_string));
    }
    return seed;
  }
};
}

#endif
