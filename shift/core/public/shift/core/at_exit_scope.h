#ifndef SHIFT_CORE_ATEXITSCOPE_H
#define SHIFT_CORE_ATEXITSCOPE_H

#include "shift/core/core.h"
#include "shift/core/types.h"

namespace shift::core
{
/// An at_exit_scope objects guarantees to call a function object on scope exit.
template <typename Function>
class at_exit_scope
{
public:
  /// Constructor.
  /// @param f
  ///   The function to be run on scope exit.
  inline at_exit_scope(Function&& f) noexcept
  : _function(std::forward<Function>(f))
  {
  }

  at_exit_scope(const at_exit_scope&) = delete;

  /// Move constructor.
  at_exit_scope(at_exit_scope&& other) : _function(std::move(other._function))
  {
    other._moved = true;
  }

  /// Destructor calling the registered function.
  inline ~at_exit_scope()
  {
    if (!_moved)
      _function();
  }

  at_exit_scope& operator=(const at_exit_scope&) = delete;

  /// Move assignment operator.
  at_exit_scope& operator=(at_exit_scope&& other)
  {
    _function = std::move(other._function);
    other._moved = true;
    return *this;
  }

private:
  Function _function;
  bool _moved = false;
};

/// Helper function making a at_exit_scope.
/// @param f
///   The function to be run on scope exit.
template <typename Function>
at_exit_scope<Function> make_at_exit_scope(Function&& f) noexcept
{
  return at_exit_scope<Function>(std::forward<Function>(f));
}
}

#endif
