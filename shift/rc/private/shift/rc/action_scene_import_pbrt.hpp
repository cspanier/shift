#ifndef SHIFT_RC_ACTION_SCENE_IMPORT_PBRT_HPP
#define SHIFT_RC_ACTION_SCENE_IMPORT_PBRT_HPP

#include <shift/core/exception.hpp>
#include <shift/resource_db/repository.hpp>
#include "shift/rc/types.hpp"

namespace shift::rc
{
struct pbrt_error : virtual core::runtime_error
{
};

class action_scene_import_pbrt
: public action_base,
  public core::singleton<action_scene_import_pbrt>
{
public:
  static constexpr const char* action_name = "scene-import-pbrt";
  static constexpr const char* action_version = "1.0.0001";

public:
  /// Default constructor.
  action_scene_import_pbrt();

  /// @see action_base::process.
  bool process(resource_compiler_impl& compiler,
               job_description& job) const override;

private:
  struct parser_context;

  bool parse_file(parser_context& context, const fs::path& base_path,
                  const fs::path& include_path,
                  const fs::path& file_path) const;

  bool parse_token_group(parser_context& context,
                         const std::vector<std::string>& tokens,
                         const fs::path& base_path,
                         const fs::path& include_path) const;
};
}

#endif
