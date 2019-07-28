# Language Reference
The following sections document all language features of [`shift.proto`](parser.proto.md).

1. [Comments](#1-comments)
2. [Identifiers](#2-identifiers)
3. [Namescopes](#3-namescopes)
4. [Structures](#4-structures)
5. [Enumerations](#5-enumerations)
6. [Aliases](#6-aliases)
7. [Constants](#7-constants)
8. [Type Paths](#8-type-paths)
9. [Attributes](#9-attributes)
10. [Templates](#10-templates)
11. [Meta Information](#11-meta-information)

## 1. Comments
Comments may be used to document the code. In contrast to other languages the `shift.proto` parser does not simply skip all comments, but processes some comments in special locations that are later forwarded into the generated C++/C# code.

### Syntax
```
comment := '#', ' '?, [^\n]*
```
The hash character '#' starts a comment. An optional single space character is always skipped, followed by the actual comment text. A comment ends with an end of line character.

### Example
```
# This comment documents usage of some_type.
struct some_type
{
  # ...
}
```

## 2. Identifiers
Each named symbol needs an identifier to store its name. Certain identifiers have strict naming rules and may only contain characters from a limited set of letters, numbers, or special characters.

### Syntax
```
identifier := [a-z], [a-z0-9_]*
template-identifier := [A-Z], [a-zA-Z0-9_]*
any-identifier := identifier | template-identifier
template-argument := type-path | uint-constant | sint-constant
```

## 3. Namescopes
`Namescope`s group other `structure`s, `enumeration`s, and `alias` definitions in a named scope to prevent naming conflicts. `Namescope`s may be nested. Multiple `namescope`s of the same name may exist within another scope. In that case all of these `namescope`s are treated as a single one. The global file scope is treated like an anonymous name scope that contains a `namescope-body`.

### Syntax

```
namescope-body := (namescope | alias | enumeration | structure)*;
namescope := meta, "namescope", identifier, '{', namescope-body, '}'
```

### Example

```
namescope foo {}

namescope my_ns
{
  struct foo {} # Ok, my_ns:foo does not collide with global symbol foo.
}

namescope my_ns
{
  # The following line would result in an error: there is already a symbol named my_ns:foo.
  # struct foo {}
}
```

## 4. Structures
`Structure`s provide one of the most important building block of the IDL. They group an ordered set of data fields into a named structure. If used in combination with [shift.tools.protogen](../../tools.protogen/doc/tools.protogen.md), each `structure` may later be serialized into a byte stream and either sent across a network or stored on disk.

### Syntax

```
field-value := "nullptr" | template-identifier | enumerant-reference | uint-constant | sint-constant
field := meta, "const"?, type-path, identifier, ('=', field-value)?
structure := meta, "struct", identifier, template-parameter-list?, (':', type-path)?,
             '{', *(field, ';'), '}'
```

### Example

```
# Base type for all shapes.
struct shape
{
  vector2<float32> center;
}

# A rectangle derives from shape.
struct rectangle : shape
{
  float32 width;
  float32 height;
}
```

## 5. Enumerations
`Enumeration`s are used to define a distinct type that may only be assigned a single value from a set of values, which is defined in a list of named constants (i.e. enumerants).

### Syntax

```
enumerant-reference := type-path, '.', identifier;
enumerant-value := ('=', template-identifier | enumerant-reference | uint-constant | sint-constant)?
enumerant := meta, identifier, enumerant-value
enumeration := meta, "enum", identifier, template-parameter-list?, ':', type-path,
               '{', (enumerant % ',')?, '}'
```

### Example

```
enum direction : var_uint_t
{
  none,        # 0
  north,       # 1
  south = 2,   # 2
  west = 4,    # 4
  east = 0x08  # 8
}
```

## 6. Aliases

### Syntax

```
alias := meta, "using", identifier, template-parameter-list?, '=', type-path, ';'
```

## 7. Constants
Currently there are only three types of constant values supported in the language. Signed and unsigned integers and character strings. They are used in a few selected locations to provide the value of constants or default parameters.

### Syntax
```
sint-constant := ("0x", hex) | ("'", char, "'") | long_long
uint-constant := ("0x", hex) | ("'", char, "'") | ulong_long
string := '"', ("\"" | [^"])*, '"'
```

## 8. Type Paths
Type paths are used in circumstances where types from different `namescope`s need to be referenced.

### Syntax
```
type-path-element := any-identifier, -('<', (template-argument % ','), '>')?
type-path := type-path-element % ':'
```

## 9. Attributes
(ToDo)

### Syntax
```
attribute := identifier, '=', string | uint-constant
attributes := '[', -(attribute % ','), ']';
```

## 10. Templates
(ToDo)

### Syntax
```
template-parameter :=
  ("typename", "..." | ' ', template-identifier, ('=', type-path | uint-constant | sint-constant)?) |
  (type-path, "..." | ' ', template-identifier, ('=', type-path | uint-constant | sint-constant)?)
template-parameter-list := '<', (template-parameter % ','), '>'
```

## 11. Meta Information
The meta information block may be attached to most language primitives and contains a series of comments and a set of attributes.

### Syntax
```
meta := comment*, attributes?
```

### Example
```
# The following two comment lines and both attributes will be
# stored with the enum type foo for later code generation.
[cpp_name="foo_t", cs_name="FooFlags"]
enum foo
{
  none,
  option1,
  option2
}
```
