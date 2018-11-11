# shift.parser.proto - A Grammar and Parser Library for the Proto Language

*This is an experimental version 2 of [shift.proto](../../proto/doc/proto.md).*

`shift.parser.proto` implements a parser for an *interface description language* (IDL) named `shift.proto`. Its syntax is intentionally similar to C-like languages but way more limited. The primary use case is to describe data structures which may be used for data exchange or storage. Use [shift.tools.protogen](../../tools.protogen/doc/tools.protogen.md) to generate C++ and/or C# code from an interface description written in `shift.proto`.

The main benefits of version 2 over the previous version are:

* Heavily improved parser error diagnostics.
* New variable sized integer types along traditional fixed size integers.
* Generalized built-in types with alias definitions to most common types written in `shift.proto`.
* Many unit tests (previous version didn't have any tests because error reporting was too limited).

## Content

* [Language Reference](language_reference.md)
* [Built-in Types](built_in_types.md)
