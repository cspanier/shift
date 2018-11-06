#ifndef SHIFT_TOOLS_PROTOGEN_GENERATORS_DOT_GENERATOR_HPP
#define SHIFT_TOOLS_PROTOGEN_GENERATORS_DOT_GENERATOR_HPP

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
/// A class to generator C++ code from a Protogen AST.
class dot_generator : public base_generator
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

  // private:
  //  /// Generates names used during code generation.
  //  void preprocess(namescope& scope);

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
  //  void gather_edges(const namescope& scope);

  //  ///
  //  void gather_edges(const node* source, const type_reference& reference);

  // private:
  //  ///
  //  struct node_pointer_comparator
  //  {
  //    bool operator()(const node* lhs, const node* rhs) const
  //    {
  //      if (!rhs)
  //        return true;
  //      if (!lhs)
  //        return false;
  //      return lhs < rhs;
  //    }
  //  };

  //  ///
  //  struct node_name_comparator
  //  {
  //    bool operator()(const node* lhs, const node* rhs) const
  //    {
  //      if (!rhs)
  //        return true;
  //      if (!lhs)
  //        return false;
  //      return lhs->name < rhs->name;
  //    }
  //  };

  //  using edge = std::pair<const node*, const node*>;

  //  /// Comparison operator used for sorting edges by pointer.
  //  struct edge_pointer_comparator : public node_pointer_comparator
  //  {
  //    bool operator()(const edge& lhs, const edge& rhs) const
  //    {
  //      return node_pointer_comparator::operator()(lhs.first, rhs.first) ||
  //             (!node_pointer_comparator::operator()(rhs.first, lhs.first) &&
  //              node_pointer_comparator::operator()(lhs.second, rhs.second));
  //    }
  //  };

  //  /// Comparison operator used for sorting edges by name.
  //  struct edge_name_comparator : public node_name_comparator
  //  {
  //    bool operator()(const edge& lhs, const edge& rhs) const
  //    {
  //      return node_name_comparator::operator()(lhs.first, rhs.first) ||
  //             (!node_name_comparator::operator()(rhs.first, lhs.first) &&
  //              node_name_comparator::operator()(lhs.second, rhs.second));
  //    }
  //  };

  //  bool _use_services;
  //  file_writer* _source;

  //  std::set<const node*, node_pointer_comparator> _nodes;
  //  std::set<edge, edge_pointer_comparator> _edges;
};
}

#endif
