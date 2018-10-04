#ifndef SHIFT_RC_ACTION_H
#define SHIFT_RC_ACTION_H

#include <string>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <chrono>
#include <regex>
#include <boost/filesystem/path.hpp>
#include <shift/core/types.h>
#include <shift/core/exception.h>
#include <shift/core/singleton.h>
#include <shift/parser/json/json.h>

namespace shift::rc
{
class resource_compiler_impl;
struct job_description;

using action_version = std::string;

/// A short description to uniquely identify each action.
struct action_description
{
  std::string name;
  action_version version;
  bool support_multithreading;
};

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

  /// Returns the static action's name.
  const std::string& name() const;

  /// Returns the static action's version.
  const action_version& version() const;

  /// Returns whether the action can be used from multiple threads.
  bool support_multithreading() const;

  /// Checks the cached version against the internal static version and marks
  /// the action as changed if the two versions don't match.
  void check_version(const action_version& cached_version);

  /// Returns whether the action itself has been modified.
  /// @remarks
  ///   The resource compiler needs to run an action not only if any input or
  ///   output files changed, but also if the algorithm itself was updated to a
  ///   new version. To detect this case each action has a version identifier.
  ///   Changing an action version invalidates all cached actions.
  bool modified() const;

  /// Process a single job.
  /// @remarks
  ///   This method may be called from multiple threads in parallel. You are
  ///   supposed to store most runtime data on the stack to avoid
  ///   synchronization issues.
  virtual bool process(resource_compiler_impl& compiler,
                       job_description& job) const = 0;

private:
  action_description _description;
  bool _modified = true;
};
}

#endif
