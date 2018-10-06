# shift.protogen - A Code Generator for [shift.proto](../../proto/doc/proto.md)

(ToDo)

# Usage

```
Allowed options:
  --help                      Shows this help message.
  --silent                    Disables logging to console.
  --no-logfile                Disables logging to file.
  --log-level arg (=warn)     Selects a log level (may be one of 'debug', 
                              'info', 'warn', or 'error')
  --log-arguments             Writes all program arguments to the log.
  --show-console arg (=1)     Show or hide the console window
  --cache-path arg            Path to store temporary data.
  --source arg                Path to a protocol definition file.
  --verbose                   Enable additional output messages.
  --strip arg                 Strip the specific name-scopes in generated code.
  --limit arg                 Limits code generation to specific name-scopes, 
                              relative to those specified using --strip.
  --exclude arg               Excludes certain name-scopes from code 
                              generation.
  -D [ --define ] arg         Preprocessor definitions.
  -f [ --force ] arg (=0)     Force code generation.
  --cpp-source-path arg       Base path where the C++ generator shall write all
                              source files to.
  --cpp-include-path arg      Base path where the C++ generator shall write all
                              include files to.
  --cpp-folder arg            Sets the folder where to generate to source code.
  --cpp-namespace arg         Set the target namespace.
  --cpp-include arg           Add include directives to each generated source 
                              file.
  --cpp-naming-convention arg Select naming convention of generated symbols. 
                              Choose between 'lower-delimited' (default) and 
                              'camel-case'.
  --cpp-clang-format arg      Path to the clang-format tool, used to 
                              automatically format the generated source code.
  --c#-path arg               Base path where the C# generator shall write to.
  --c#-namespace arg          Set the target namespace
  --c#-using arg              Add using directives to each generated source 
                              file.
  --dot-path arg              Base path where the GraphViz generator shall 
                              write to.
  --proto-path arg            Base path where the Proto generator shall write 
                              to.
```
