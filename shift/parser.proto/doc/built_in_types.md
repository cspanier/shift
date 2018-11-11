# Built-in Types
[`shift.proto`](parser.proto.md) defines a minimal set of built-in types that may be used to build data structures. The type system is heavily extendible through use of alias definitions and attributes that are later used in the target code generators.

## Fundamental Types
```
fixed_sint_t<N>      # (1)
fixed_uint_t<N>      # (2)
var_sint_t           # (3)
var_uint_t           # (4)
floating_point_t<N>  # (5)
repeated_t<T, N>     # (6)
selected_t<S, Ts...>  # (7)
optional_t<S, Ts...>  # (8)
```

(1-2) Fixed size signed (1) and unsigned (2) integer types do have a template parameter `N`, which must be a constant integer expression denoting the size in bits. `N` may be set to one of 8, 16, 32, or 64.

(3-4) Variable sized signed (3) and unsigned (4) integers may be encoded in a more compact way than regular fixed size integers and are particularly useful if you expect its values *usually* smaller than the maximum size of the integer type.

(5) A single built-in floating-point type defined as a template type. The template argument N may be assigned 16 (for 16 bit half precision), 32 (for 32 bit default precision), or 64 (for 64 bit double precision).

(6) Repeats the same record type `T` either a constant or a variable number of times `N`. The template arguement `T` may be any type and `N` may be either any unsigned integer type (2) or (4), or an unsigned integer constant.

(7) The unsigned integer type `S` serving as an index into the list of types `Ts...` selects which of the types is present. All remaining types in `Ts...` are skipped.

(8) A bit field selector `S` selects which of the following types in `Ts...` are present or skipped. This currently limits the maximum number of types in `Ts...` to 64.

## Alias Types
The following convenient alias types ease usage of the available fundamental types:

```
# Fixed size signed integer types:
using sfixed8 = fixed_sint_t<8>;       # (1)
using sfixed16 = fixed_sint_t<16>;     # (2)
using sfixed32 = fixed_sint_t<32>;     # (3)
using sfixed64 = fixed_sint_t<64>;     # (4)

# Fixed size unsigned integer types:
using ufixed8 = fixed_uint_t<8>;       # (5)
using ufixed16 = fixed_uint_t<16>;     # (6)
using ufixed32 = fixed_uint_t<32>;     # (7)
using ufixed64 = fixed_uint_t<64>;     # (8)

# Variable size signed integer types:
using sint8 = sfixed8;                 # (9)
using sint16 = var_sint_t;             # (10)
using sint32 = var_sint_t;             # (11)
using sint64 = var_sint_t;             # (12)

# Variable size unsigned integer types:
using uint8 = ufixed8;                 # (13)
using uint16 = var_uint_t;             # (14)
using uint32 = var_uint_t;             # (15)
using uint64 = var_uint_t;             # (16)

# Boolean Type
using bool = fixed_uint_t<8>;          # (17)

# Character Types
using char8 = fixed_uint_t<8>;         # (18)
using char16 = var_uint_t;             # (19)
using char32 = var_uint_t;             # (20)

# Floating-point Types
using float16 = floating_point_t<16>;  # (21)
using float32 = floating_point_t<32>;  # (22)
using float64 = floating_point_t<64>;  # (23)

# String Types
using string8 = repeated_t<char8>;     # (24)
using string16 = repeated_t<char16>;   # (25)
```

(9,11) These 8 bit variable size integers are actually fixed size integers.

(17) There is no difference between boolean and integer types in `shift.proto`. Instead, each boolean value is encoded as an 8 bit fixed unsigned integer with a value of zero being interpreted as false and any other value being interpreted as true.

(18-20) Characters are treated as variable size unsigned integer types containing arbitrary data.

(21) Half precision floating-point type.

(22) Standard precision floating-point type.

(23) Double precision floating-point type.
