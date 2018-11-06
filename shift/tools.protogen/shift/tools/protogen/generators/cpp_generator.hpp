#ifndef SHIFT_TOOLS_PROTOGEN_GENERATORS_CPP_GENERATOR_HPP
#define SHIFT_TOOLS_PROTOGEN_GENERATORS_CPP_GENERATOR_HPP

#include <map>
#include <set>
#include <functional>
#include <string>
#include <boost/program_options.hpp>
#include "shift/tools/protogen/generators/base_generator.hpp"

namespace shift::tools::protogen
{
class file_writer;
class translator;
struct message;
struct namescope;
struct node;
}

namespace shift::tools::protogen::generators
{
///
struct node_comparator
{
  bool operator()(const node* lhs, const node* rhs) const;
};

using node_set = std::set<const node*, node_comparator>;

/// A class to generator C++ code from a Protogen AST.
class cpp_generator : public base_generator
{
  // public:
  //  /// @see base_generator::active.
  //  virtual bool active() const override;

  //  /// @see base_generator::output_paths.
  //  virtual std::vector<boost::filesystem::path> output_paths() const
  //  override;

  //  /// The main entry point for the code generator.
  //  virtual bool generate(namescope& root_scope, namescope& limit_scope)
  //  override;

  //  /// A wrapper for base_generator::relative_path to ease use.
  //  static std::string relative_name(const node& node,
  //                                   const namescope* current_scope,
  //                                   std::string delimiter = "::");

  // private:
  //  /// Generates names used during code generation.
  //  void preprocess(namescope& scope);

  //  /// Checks whether a message is copy constructable.
  //  bool is_copy_constructable(const message& message);

  //  /// Checks whether a type is copy constructable.
  //  bool is_copy_constructable(const type_reference& reference);

  //  /// Write a copy constructor or copy assignment method body.
  //  void write_copy_ctor_or_copy_assignment_body(const message& message);

  //  /// Write a move constructor or move assignment method body.
  //  void write_move_ctor_or_move_assignment_body(const message& message);

  //  ///
  //  void gather_nodes(const namescope& scope);

  //  ///
  //  void gather_nodes(const type_reference& reference);

  //  ///
  //  void gather_nodes(const alias& alias);

  //  ///
  //  void gather_nodes(const enumeration& enumeration);

  //  ///
  //  void gather_nodes(const message& message);

  //  ///
  //  void gather_nodes(const interface& interface);

  //  ///
  //  void gather_nodes(const service& service);

  //  ///
  //  void gather_dependencies(const node& target);

  //  ///
  //  void gather_dependencies(const node& target, const type_reference&
  //  reference,
  //                           node_set& required_declarations,
  //                           node_set& required_definitions, bool definition);

  //  ///
  //  void gather_dependencies(const node& target, const alias& alias,
  //                           node_set& required_declarations,
  //                           node_set& required_definitions, bool definition);

  //  ///
  //  void gather_dependencies(const node& target, const enumeration&
  //  enumeration,
  //                           node_set& required_declarations,
  //                           node_set& required_definitions, bool definition);

  //  ///
  //  void gather_dependencies(const node& target, const message& message,
  //                           node_set& required_declarations,
  //                           node_set& required_definitions, bool definition);

  //  ///
  //  void gather_dependencies(const node& target, const interface& interface,
  //                           node_set& required_declarations,
  //                           node_set& required_definitions, bool definition);

  //  ///
  //  void gather_dependencies(const node& target, const service& service,
  //                           node_set& required_declarations,
  //                           node_set& required_definitions, bool definition);

  //  ///
  //  void write_declaration(const node& node);

  //  ///
  //  void write_declaration(const alias& alias);

  //  ///
  //  void write_declaration(const enumeration& enumeration);

  //  ///
  //  void write_declaration(const message& message);

  //  ///
  //  void write_declaration(const interface& interface);

  //  ///
  //  void write_declaration(const service& service);

  //  ///
  //  void write_definition(const node& node);

  //  ///
  //  void write_definition(const alias& alias);

  //  ///
  //  void write_definition(const enumeration& enumeration);

  //  ///
  //  void write_definition(const message& message);

  //  ///
  //  void write_definition(const interface& interface);

  //  ///
  //  void write_definition(const service& service);

  //  ///
  //  std::string def_group_name(const node& node);

  //  ///
  //  std::string decl_group_name(const node& node);

  // private:
  //  enum class naming_convention
  //  {
  //    lower_delimited,
  //    camel_case
  //  };

  //  bool _use_services;
  //  file_writer* _header;
  //  file_writer* _source;
  //  bool _use_header;
  //  bool _use_source;
  //  naming_convention _naming_convention = naming_convention::lower_delimited;

  //  node_set _nodes;
  //  std::multimap<std::string, std::pair<const node*, bool>> _node_groups;
  //  std::map<const node*, node_set, node_comparator> _declarations;
  //  std::map<const node*, node_set, node_comparator> _definitions;
};
}

#endif
