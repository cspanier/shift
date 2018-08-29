#ifndef PROTOGEN_GENERATOR_CSGENERATOR_H
#define PROTOGEN_GENERATOR_CSGENERATOR_H

#include <string>
#include <shift/core/boost_disable_warnings.h>
#include <boost/program_options.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "shift/protogen/generator/base_generator.h"
#include "shift/protogen/file_writer.h"

namespace shift::proto::generator
{
/// A class to generator C# code from a Protogen AST.
class cs_generator : public base_generator
{
public:
  /// @see base_generator::active.
  bool active() const override;

  /// @see base_generator::output_paths.
  std::vector<boost::filesystem::path> output_paths() const override;

  /// The main entry point for the code generator.
  bool generate(namescope& root_scope, namescope& limit_scope) override;

  /// A wrapper for base_generator::relative_path to ease use.
  static std::string relative_name(const node& node,
                                   const namescope& current_scope,
                                   std::string delimiter = ".");

private:
  /// Generates names used during code generation.
  void preprocess(namescope& scope);

  /// Writes enumeration definitions.
  void writeEnumDefinitions(namescope& scope);

  /// Writes alias definitions.
  void writeAliasDefinitions(namescope& scope);

  /// Recursively finds and writes all messages.
  void writeMessageDefinitions(namescope& scope);

  ///
  bool writeMessageAsArgumentList(namescope& scope, message& message,
                                  bool firstField);

  ///
  bool writeMessageAsParameterList(namescope& scope, message& message,
                                   bool firstField);

  /// Recursively finds and writes all interfaces.
  void writeInterfaceDefinitions(namescope& scope);

  /// Checks whether there are any interface serializers that need to be
  /// written.
  bool requiresInterfaceSerializers(namescope& scope);

  /// Checks whether there are any interface deserializers that need to be
  /// written.
  bool requiresInterfaceDeserializers(namescope& scope);

  /// Writes interface method deserializers.
  void writeInterfaceDeserializers(namescope& scope);

  /// Checks whether there are any service definitions that need to be
  /// written.
  bool requiresServiceDefinitions(namescope& scope);

  /// Write service classes.
  void writeServices(namescope& scope);

  /// Checks whether the type registry needs to be written.
  bool requiresTypeRegistry(namescope& scope);

  /// Write type registration calls.
  void writeTypeRegistry(namescope& scope);

  file_writer* _source;
};
}

#endif
