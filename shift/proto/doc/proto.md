# shift.proto - A Grammar and Parser Library for the Proto Language

`shift.proto` is an *interface description language* (IDL). Its syntax is similar to C/C++ but way more limited. The primary use case is to describe data structures packed in messages exchanged between programs. Messages can optionally be organized in interfaces and two interfaces can be used to describe a [service](../../service/doc/service.md) client/provider pair. Use [shift.protogen](../../protogen/doc/protogen.md) to generate C++ and/or C# code from an interface description written in `shift.proto`.

## Language Grammar

```
sint := ("0x", hex) | ("'", char, "'") | long_long
uint := ("0x", hex) | ("'", char, "'") | ulong_long

namescope_name := [a-z], [a-zA-Z0-9_]*
typename := [a-zA-Z], [a-zA-Z0-9_]*
interface_name := [Ii], typename
type_path := ((namescope_name | typename | interface_name), '.')*, typename
interface_path := (namescope_name, '.')*, interface_name
identifier := [a-z], [a-zA-Z0-9_]*
string := '"', ("\"" | [^"])*, '"'

bool_type := "bool"
char_type := "char8" | "char16" | "char32"
sint_type := "int8" | "int16" | "int32" | "int64"
uint_type := "uint8" | "uint16" | "uint32" | "uint64" | "datetime"
int_type := sint_type | uint_type
float_type := "float32" | "float64"
string_type := "string"
template_type := "raw_ptr" | "unique_ptr" | "shared_ptr" | "weak_ptr" | "group_ptr" | "tuple" |
                 "array" | "list" | "vector" | "set" | "matrix" | "map" | "variant" | "bitfield"
built_in_type := bool_type | char_type | int_type | float_type | string_type | template_type
service_type := "service_client" | "service_provider"

attribute := identifier, ('=', (string | uint))?
attributes := '[', (attribute % ',')?, ']'
type_reference := (built_in_type | type_path), ('<', ((type_reference | int) % ','), '>')?
alias := attributes?, "using", typename, '=', type_reference, ';'
enumerator := attributes?, identifier, ('=', sint)?
enumeration := attributes?, "enum", typename, ':', type_reference, '{', (enumerator % ',')?, '}'
field := attributes?, type_reference, identifier
message := attributes?, ("message" | "struct"), typename,
           ("extends", type_path)?, '{', (field, ';')*, '}'
interface := attributes?, "interface", interface_name, '{', message*, '}'
service := attributes?, service_type, '<', interface_path, ',', interface_path, '>', typename, ';'
scope := (namescope | alias | enumeration | message | interface | service)*
namescope := attributes?, "namescope", namescope_name, '{', scope, '}'
```

## Examples

```
namescope sample1
{
  message Ping
  {
    uint64 clientTime;
  }

  message Pong
  {
    uint64 clientTime;
    uint64 providerTime;
  }
}
```
This simple example defines two messages `Ping` and `Pong` containing a couple of data fields. `protogen` would generate the messages along with appropriate de-/serialization code.

```
namescope sample2
{
  interface IPingPongProvider
  {
    message Ping
    {
      uint64 clientTime;
    }
  }

  interface IPingPongClient
  {
    message Pong
    {
      uint64 clientTime;
      uint64 providerTime;
    }
  }

  service_client<IPingPongProvider, IPingPongClient> PingPongClient;
  service_provider<IPingPongProvider, IPingPongClient> PingPongProvider;
}
```
This slightly extended example lets `protogen` generate the service provider `PingPongProvider` and the service client `PingPongClient`. A client could send a `Ping` message to a provider and the provider would reply with `Pong`.