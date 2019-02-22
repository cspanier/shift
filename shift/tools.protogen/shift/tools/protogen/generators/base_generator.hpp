#ifndef SHIFT_TOOLS_PROTOGEN_GENERATORS_BASE_GENERATOR_HPP
#define SHIFT_TOOLS_PROTOGEN_GENERATORS_BASE_GENERATOR_HPP

#include <sstream>
#include <functional>
#include <filesystem>
#include <shift/parser/proto/proto.hpp>

namespace shift::tools::protogen
{
class translator;
}

namespace shift::tools::protogen::generators
{
///
class base_generator
{
public:
  //  using name_lookup =
  //    std::function<std::string(const parser::proto::ast::type_node& node)>;

  //  virtual ~base_generator() = 0;

  //  /// Returns whether the generator is activated through command line
  //  /// arguments.
  //  virtual bool active() const = 0;

  //  /// Returns all generator's output paths.
  //  virtual std::vector<std::filesystem::path> output_paths() const = 0;

  //  ///
  //  virtual bool generate(parser::proto::ast::namescope_node& root_scope,
  //                        parser::proto::ast::namescope_node& limit_scope) =
  //                        0;

  //  /// A common helper function to get a type path that is sufficiently exact
  //  /// from the current name scope.
  //  static parser::proto::ast::type_path relative_path(
  //    const base_generator::name_lookup& name_lookup,
  //    const parser::proto::ast::type_node* node,
  //    const parser::proto::ast::namescope_node* current_scope = nullptr);

  //  /// A simple helper function to create a pair of name scope and type
  //  /// reference.
  //  static std::pair<const parser::proto::ast::namescope_node&,
  //                   const parser::proto::ast::type_reference_node&>
  //  make_scope_and_type(
  //    const parser::proto::ast::namescope_node& namescope,
  //    const parser::proto::ast::type_reference_node& type_reference);

  // protected:
  //  /// Changes a name to delimiter-separated lower case (e.g.
  //  /// "delimiter_case").
  //  static std::string delimiter_case(std::string name);

  //  /// Changes a name to pascal case (e.g. "PascalCase").
  //  static std::string pascal_case(std::string name);

  //  /// Changes a name to camel case (e.g. "camelCase").
  //  static std::string camel_case(std::string name);

  //  /// Changes a name to upper case (e.g. "UPPERCASE").
  //  static std::string upper_case(std::string name);

  //  /// Changes a name to lower case (e.g. "lowercase").
  //  static std::string lower_case(std::string name);
};
}

#endif
