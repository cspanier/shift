#ifndef SHIFT_RC_ACTION_SHADER_COMPILE_HPP
#define SHIFT_RC_ACTION_SHADER_COMPILE_HPP

#include <shift/resource_db/repository.hpp>
#include "shift/rc/types.hpp"

namespace shift::rc
{
class action_shader_compile : public action_base,
                              public core::singleton<action_shader_compile>
{
public:
  static constexpr const char* action_name = "shader-compile";
  static constexpr const char* action_version = "1.0.0003";

public:
  /// Default constructor.
  action_shader_compile();

  /// @see action_base::process.
  bool process(resource_compiler_impl& compiler,
               job_description& job) const override;
};
}

#endif
