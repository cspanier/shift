# shift.parser.proto - A Grammar and Parser Library for the Proto Language

*This is an experimental version 2 of [shift.proto](../../proto/doc/proto.md).*

`shift.parser.proto` implements a parser for an *interface description language* (IDL) named `shift.proto`. Its syntax is intentionally similar to C-like languages but way more limited. The primary use case is to describe data structures packed in messages that can be exchanged between programs. Use [shift.tools.protogen](../../tools.protogen/doc/tools.protogen.md) to generate C++ and/or C# code from an interface description written in `shift.proto`.

## 1. Language Reference

### 1.1 Namescopes
`Namescope`s group other `structure`s, `enumeration`s, and `alias` definitions in a named scope to prevent naming conflicts. `Namescope`s may be nested. Multiple `namescope`s of the same name may exist within another scope. In that case all of these `namescope`s are treated as a single one. The global file scope is treated like an anonymous name scope that contains a `namescope-body`.

#### Syntax

```
namescope-body := (namescope | alias | enumeration | structure)*;
namescope := meta, "namescope", identifier, '{', namescope-body, '}'
```

### 1.2 Structures
`Structure`s provide one of the most important building block of the IDL. They group an ordered set of data fields into a named structure. If used in combination with [shift.tools.protogen](../../tools.protogen/doc/tools.protogen.md), each `structure` may later be serialized into a byte stream and either sent across a network or stored on disk.

#### Syntax

```
field-value := ('=', "nullptr" | template-identifier | enumerant-reference | uint-constant | sint-constant)?
field := meta, "const"?, type-path, identifier, field-value
structure := meta, "struct", identifier, template-parameters?, (':', type-path)?,
             '{', *(field, ';'), '}'
```

### 1.3 Enumerations
`Enumeration`s are used to define a distinct type that may only be assigned a single value from a set of values, which is defined in a list of named constants (i.e. enumerants).

#### Syntax

```
enumerant-reference := type-path, '.', identifier;
enumerant-value := ('=', template-identifier | enumerant-reference | uint-constant | sint-constant)?
enumerant := meta, identifier, enumerant-value
enumeration := meta, "enum", identifier, template-parameters?, ':', type-path,
               '{', (enumerant % ',')?, '}'
```

### 1.4 Aliases

#### Syntax

```
alias := meta, "using", identifier, template-parameters?, '=', type-path, ';'
```

### 1.5 Constants
Currently there are only three types of constant values supported in the language. Signed and unsigned integers and character strings. They are used in a few selected locations to provide the value of constants or default parameters.

#### Syntax
```
sint-constant := ("0x", hex) | ("'", char, "'") | long_long
uint-constant := ("0x", hex) | ("'", char, "'") | ulong_long
string := '"', ("\"" | [^"])*, '"'
```

### 1.6 Identifiers
Each named symbol needs an identifier to store its name. Certain identifiers have strict naming rules and may only contain characters from a limited set of letters, numbers, or special characters.

#### Syntax
```
identifier := [a-z], [a-z0-9_]*
template-identifier := [A-Z], [a-zA-Z0-9_]*
any-identifier := identifier | template-identifier
template-argument := type-path | uint-constant | sint-constant
```

### 1.7 Type Paths
Type paths are used in circumstances where types from different `namescope`s need to be referenced.

#### Syntax
```
type-path-element := any-identifier, -('<', (template-argument % ','), '>')?
type-path := type-path-element % ':'
```

### 1.8 Attributes
(ToDo)

#### Syntax
```
attribute := identifier, '=', string | uint-constant
attributes := '[', -(attribute % ','), ']';
```

### 1.9 Templates
(ToDo)

#### Syntax
```
template-parameter :=
  ("typename", "..." | ' ', template-identifier, ('=', type-path | uint-constant | sint-constant)?) |
  (type-path, "..." | ' ', template-identifier, ('=', type-path | uint-constant | sint-constant)?)
template-parameters := '<', template-parameter % ',', '>'
```

### 1.10 Meta Information
(ToDo)

#### Syntax
```
comment := '#', ' '?, [^\n]*
meta := comment*, attributes?
```
