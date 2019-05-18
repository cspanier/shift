#include "shift/core/error_condition.hpp"
#include <boost/assert.hpp>

namespace shift::core
{
///
class error_condition_category : std::error_category
{
public:
  const char* name() const noexcept override;

  std::string message(int condition) const override;

private:
  error_condition_category() = default;

  friend std::error_condition make_error_condition(error_condition condition);
};

const char* error_condition_category::name() const noexcept
{
  return "shift::image::error_condition";
}

std::string error_condition_category::message(int condition) const
{
  switch (static_cast<error_condition>(condition))
  {
  case error_condition::invalid_argument:
    return "Invalid function arguments.";

  case error_condition::operation_not_supported:
    return "Operation not supported.";

  case error_condition::internal_error:
    return "Internal error occurred.";
  }

  BOOST_ASSERT(false);
  return "(unrecognized condition)";
}

std::error_condition make_error_condition(error_condition condition)
{
  static const error_condition_category category{};
  return {static_cast<int>(condition), category};
}
}
