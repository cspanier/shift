#include "shift/livedebug/error_code.hpp"
#include <boost/assert.hpp>

namespace shift::livedebug
{
///
class error_code_category : std::error_category
{
public:
  const char* name() const noexcept override;

  std::string message(int code) const override;

  bool equivalent(int code, const std::error_condition& condition) const
    noexcept override;

private:
  error_code_category() = default;

  friend std::error_code make_error_code(error_code code);
};

const char* error_code_category::name() const noexcept
{
  return "shift::image::error_code";
}

std::string error_code_category::message(int code) const
{
  switch (static_cast<error_code>(code))
  {
  case error_code::cannot_create_socket:
    return "Cannot create new network socket.";
  case error_code::endpoint_already_in_use:
    return "IP endpoint already in use.";
  case error_code::cannot_reuse_address:
    return "Cannot set reuse address option on socket.";
  case error_code::cannot_bind_to_endpoint:
    return "Cannot bind socket to specified endpoint.";
  case error_code::cannot_listen:
    return "Cannot listen on socket.";
  case error_code::internal_error:
    return "Internal error.";
  }

  BOOST_ASSERT(false);
  return "(unrecognized error)";
}

bool error_code_category::equivalent(
  int code, const std::error_condition& condition) const noexcept
{
  using shift::core::error_condition;

  static const auto& error_condition_category =
    std::error_condition{error_condition{}}.category();

  if (condition.category() != error_condition_category)
    return false;

  switch (static_cast<error_condition>(condition.value()))
  {
  case error_condition::invalid_argument:
    return code >= static_cast<int>(error_condition::invalid_argument) &&
           code < static_cast<int>(error_condition::operation_not_supported);

  case error_condition::operation_not_supported:
    return code >= static_cast<int>(error_condition::operation_not_supported) &&
           code < static_cast<int>(error_condition::internal_error);

  case error_condition::internal_error:
    return code >= static_cast<int>(error_condition::internal_error);
  }

  BOOST_ASSERT(false);
  return false;
}

std::error_code make_error_code(error_code code)
{
  static const error_code_category category{};
  return {static_cast<int>(code), category};
}
}
