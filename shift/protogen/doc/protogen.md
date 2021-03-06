# shift.protogen - A Code Generator for [shift.proto](../../proto/doc/proto.md)

(ToDo)

# Usage

```
Allowed options:
  --help                        Shows this help message.
  --silent                      Disables logging to console.
  --no-logfile                  Disables logging to file.
  --log-level arg (=warn)       Selects a log level (may be one of 'debug', 
                                'info', 'warn', or 'error')
  --log-arguments               Writes all program arguments to the log.
  --show-console arg (=1)       Show or hide the console window
  --cache-path arg              Path to store temporary data.
  --source arg                  Path to a protocol definition file.
  --verbose                     Enable additional output messages.
  --strip arg                   Strip the specific name-scopes in generated 
                                code.
  --limit arg                   Limits code generation to specific name-scopes,
                                relative to those specified using --strip.
  --exclude arg                 Excludes certain name-scopes from code 
                                generation.
  -D [ --define ] arg           Preprocessor definitions.
  -f [ --force ] arg (=0)       Force code generation.
  --cpp-source-path arg         Base path where the C++ generator shall write 
                                all source files to.
  --cpp-include-path arg        Base path where the C++ generator shall write 
                                all include files to.
  --cpp-folder arg              Sets the folder where to generate to source 
                                code.
  --cpp-namespace arg           Set the target namespace.
  --cpp-include arg             Add include directives to each generated source
                                file.
  --cpp-naming-convention arg   Select naming convention of generated symbols. 
                                Choose between 'lower-delimited' (default) and 
                                'camel-case'.
  --cpp-indent-width arg (=2)   The number of spaces to use to indent C++ code.
  --cpp-clang-format arg        Path to the clang-format tool, used to 
                                automatically format the generated source code.
  --c#-path arg                 Base path where the C# generator shall write 
                                to.
  --c#-namespace arg            Set the target namespace
  --c#-using arg                Add using directives to each generated source 
                                file.
  --cs-indent-width arg (=2)    The number of spaces to use to indent C# code.
  --dot-path arg                Base path where the GraphViz generator shall 
                                write to.
  --dot-indent-width arg (=2)   The number of spaces to use to indent Graphviz 
                                code.
  --proto-path arg              Base path where the Proto generator shall write
                                to.
  --proto-indent-width arg (=2) The number of spaces to use to indent proto 
                                code.

```

## CMake Integration

The CMake script `cmake/ShiftAddProtogen.cmake` provides the macro `shift_add_protogen` to add a new target that calls `protogen`:

```
shift_add_protogen(protogen_target_name
  PROTO_PATH "${CMAKE_CURRENT_SOURCE_DIR}"
  CPP_SOURCE_PATH "${CMAKE_SOURCE_DIR}/cpp/source"
  CPP_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/cpp/include"
  CPP_NAMESPACE "some::namespace"
  CS_PATH "${CMAKE_SOURCE_DIR}/cs"
  CS_NAMESPACE "Some.Namespace"
)
```
The source files generated by `protogen` can be used to create a static C++ library or C# assembly. You should make sure that any targets dependent on the files generated by `protogen` depend on `protogen_target_name`:
```
add_dependencies(my_static_lib protogen_target_name)
```
