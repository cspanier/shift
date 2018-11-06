#ifndef SHIFT_TOOLS_PROTOGEN_GENERATORS_PROTO_GENERATOR_HPP
#define SHIFT_TOOLS_PROTOGEN_GENERATORS_PROTO_GENERATOR_HPP

#include <shift/core/boost_disable_warnings.hpp>
#include <boost/program_options.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/tools/protogen/generators/base_generator.hpp"
#include "shift/tools/protogen/file_writer.hpp"

namespace shift::tools::protogen::generators
{
///
class proto_generator : public base_generator
{
  // public:
  //  /// @see base_generator::active.
  //  virtual bool active() const override;

  //  /// @see base_generator::output_paths.
  //  virtual std::vector<boost::filesystem::path> output_paths() const
  //  override;

  //  ///
  //  bool generate(namescope& root_scope, namescope& limit_scope) override;

  //  /// A wrapper for base_generator::relative_path to ease use.
  //  static std::string relative_name(const node* node,
  //                                   const namescope* current_scope);

  // private:
  //  ///
  //  void write(std::string_view source_filename, const namescope& scope);

  //  ///
  //  void write(const alias& alias);

  //  ///
  //  void write(const enumeration& enumeration);

  //  ///
  //  void write(const message& message);

  //  ///
  //  void write(std::string_view source_filename, const interface& interface);

  //  ///
  //  void write(const service& service);

  //  ///
  //  void write_attributes(const node& node);

  //  file_writer _source;
};
}

#endif
