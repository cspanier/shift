## Built-in Types
`shift.proto` defines a minimal set of built-in types that may be used to build data structures. The type system is heavily extendible through use of attributes that are later used in the target code generators.

### Fixed Size Integer Types
```
1) fixed_sint_t<N>
2) fixed_uint_t<N>
```
Both signed (1) and unsigned (2) fixed size integer types do have a template parameter N, which may be set to one of 8, 16, 32, or 64. The following convenient types ease usage of these types:
```
using sfixed8 = fixed_sint_t<8>;
using sfixed16 = fixed_sint_t<16>;
using sfixed32 = fixed_sint_t<32>;
using sfixed64 = fixed_sint_t<64>;

using ufixed8 = fixed_uint_t<8>;
using ufixed16 = fixed_uint_t<16>;
using ufixed32 = fixed_uint_t<32>;
using ufixed64 = fixed_uint_t<64>;
```

### Variable Size Integer Types
```
1) var_sint_t
2) var_uint_t
```
Variable sized integers may be encoded in a more compact way than regular fixed size integers and are particularly useful if you expect its values *usually* smaller than the maximum size of the integer type. The following convenient types ease usage of these types:
```
using sint8 = sfixed8;
using sint16 = var_sint_t;
using sint32 = var_sint_t;
using sint64 = var_sint_t;
using uint8 = ufixed8;
using uint16 = var_uint_t;
using uint32 = var_uint_t;
using uint64 = var_uint_t;
```
Note that 8 bit variable sized integers are actually fixed size integers.

### Boolean Type
There is no difference between boolean and integer types in `shift.proto`. Instead, each boolean value is encoded as an 8 bit fixed unsigned integer with a value of zero being interpreted as false and any other value being interpreted as true. The type is defined as:
```
using bool = fixed_uint_t<8>;
```

### Character Types
There are no specialized character types either, but the following alias types are defined for convenience:
```
using char8 = fixed_uint_t<8>;
using char16 = var_uint_t;
using char32 = var_uint_t;
```

### Floating-Point Types
There is only a single built-in floating-point type defined as a template type:
```
1) floating_point_t<N>
```
The template argument N may be assigned 16 (for 16 bit half precision), 32 (for 32 bit default precision), or 64 (for 64 bit double precision). The following convenient types ease usage of the floating-point type:
```
using float16 = floating_point_t<16>;
using float32 = floating_point_t<32>;
using float64 = floating_point_t<64>;
```
### String Types
```
using string8 = repeated_t<char8>;
using string16 = repeated_t<char16>;
```
