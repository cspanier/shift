#ifndef PROTOGEN_GENERATOR_BASEGENERATOR_H
#define PROTOGEN_GENERATOR_BASEGENERATOR_H

#include <sstream>
#include <functional>
#include <filesystem>
#include <shift/proto/proto.hpp>
#include "shift/protogen/program_options.hpp"

namespace shift::proto
{
class translator;
}

namespace shift::proto::generator
{
///
class base_generator : public program_options
{
public:
  using name_lookup = std::function<std::string(const node& node)>;

  virtual ~base_generator() = 0;

  /// Returns whether the generator is activated through command line
  /// arguments.
  virtual bool active() const = 0;

  /// Returns all generator's output paths.
  virtual std::vector<std::filesystem::path> output_paths() const = 0;

  ///
  virtual bool generate(namescope& root_scope, namescope& limit_scope) = 0;

  /// A common helper function to get a type path that is sufficiently exact
  /// from the current name scope.
  static type_path relative_path(const base_generator::name_lookup& name_lookup,
                                 const node* node,
                                 const namescope* current_scope = nullptr);

  /// A simple helper function to create a pair of name scope and type
  /// reference.
  static std::pair<const namescope&, const type_reference&> make_scope_and_type(
    const namescope& namescope, const type_reference& type_reference);

protected:
  /// Changes a name to delimiter-separated lower case (e.g.
  /// "delimiter_case").
  static std::string delimiter_case(std::string name);

  /// Changes a name to pascal case (e.g. "PascalCase").
  static std::string pascal_case(std::string name);

  /// Changes a name to camel case (e.g. "camelCase").
  static std::string camel_case(std::string name);

  /// Changes a name to upper case (e.g. "UPPERCASE").
  static std::string upper_case(std::string name);

  /// Changes a name to lower case (e.g. "lowercase").
  static std::string lower_case(std::string name);
};
}

#endif
