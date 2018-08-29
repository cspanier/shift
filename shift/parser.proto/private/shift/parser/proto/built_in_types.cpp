#include "shift/parser/proto/built_in_types.h"

namespace shift::parser::proto
{
std::shared_ptr<source_module> built_in_types::source =
  std::make_shared<source_module>(R"(# Source of built-in types.
# Standard fixed size integer types.
using sfixed8 = fixed_sint_t<8>;
using sfixed16 = fixed_sint_t<16>;
using sfixed32 = fixed_sint_t<32>;
using sfixed64 = fixed_sint_t<64>;

using ufixed8 = fixed_uint_t<8>;
using ufixed16 = fixed_uint_t<16>;
using ufixed32 = fixed_uint_t<32>;
using ufixed64 = fixed_uint_t<64>;

# Standard variable size integer types.
# Note that 8 bit variable sized integers are actually fixed size.
using sint8 = sfixed8;
using sint16 = var_sint_t;
using sint32 = var_sint_t;
using sint64 = var_sint_t;
using uint8 = ufixed8;
using uint16 = var_uint_t;
using uint32 = var_uint_t;
using uint64 = var_uint_t;

# Boolean type.
using bool = fixed_uint_t<8>;

# Character types.
using char8 = fixed_uint_t<8>;
using char16 = var_uint_t;
using char32 = var_uint_t;

# Floating-point types.
using float16 = floating_point_t<16>;
using float32 = floating_point_t<32>;
using float64 = floating_point_t<64>;

# String types.
using string8 = repeated_t<char8>;
using string16 = repeated_t<char16>;

namescope math
{
  # Mathematical vector types.
  using vector<typename T, ufixed8 N> = repeated_t<T, N>;
  using vector2<typename T> = vector<T, 2>;
  using vector3<typename T> = vector<T, 3>;
  using vector4<typename T> = vector<T, 4>;
  using vector2f16 = vector2<float16>;
  using vector3f16 = vector3<float16>;
  using vector4f16 = vector4<float16>;
  using vector2f32 = vector2<float32>;
  using vector3f32 = vector3<float32>;
  using vector4f32 = vector4<float32>;
  using vector2f64 = vector2<float64>;
  using vector3f64 = vector3<float64>;
  using vector4f64 = vector4<float64>;

  ## Alternative definition of vector types.
  # using vector2f16 = repeated_t<float16, 2>;
  # using vector3f16 = repeated_t<float16, 3>;
  # using vector4f16 = repeated_t<float16, 4>;
  # using vector2f32 = repeated_t<float32, 2>;
  # using vector3f32 = repeated_t<float32, 3>;
  # using vector4f32 = repeated_t<float32, 4>;
  # using vector2f64 = repeated_t<float64, 2>;
  # using vector3f64 = repeated_t<float64, 3>;
  # using vector4f64 = repeated_t<float64, 4>;

  # Mathematical matrix types.
  ## ToDo: The following line requires evaluation of constant expressions.
  # using matrix<typename T, uint8 Rows, uint8 Columns> =
  #   repeated_t<T, Rows * Columns>;
  # using matrix22<typename T> = matrix<T, 2, 2>;
  # using matrix33<typename T> = matrix<T, 3, 3>;
  # using matrix44<typename T> = matrix<T, 4, 4>;
  # using matrix22<typename T> = repeated_t<T, 4>;
  # using matrix33<typename T> = repeated_t<T, 9>;
  # using matrix44<typename T> = repeated_t<T, 16>;
  # using matrix22f16 = matrix22<float16>;
  # using matrix33f16 = matrix33<float16>;
  # using matrix44f16 = matrix44<float16>;
  # using matrix22f32 = matrix22<float32>;
  # using matrix33f32 = matrix33<float32>;
  # using matrix44f32 = matrix44<float32>;
  # using matrix22f64 = matrix22<float64>;
  # using matrix33f64 = matrix33<float64>;
  # using matrix44f64 = matrix44<float64>;

  ## Alternative definition of matrix types.
  using matrix22f16 = repeated_t<float16, 4>;
  using matrix33f16 = repeated_t<float16, 9>;
  using matrix44f16 = repeated_t<float16, 16>;
  using matrix22f32 = repeated_t<float32, 4>;
  using matrix33f32 = repeated_t<float32, 9>;
  using matrix44f32 = repeated_t<float32, 16>;
  using matrix22f64 = repeated_t<float64, 4>;
  using matrix33f64 = repeated_t<float64, 9>;
  using matrix44f64 = repeated_t<float64, 16>;
}
)",
                                  "built_in_types.pro2", 0);
}
