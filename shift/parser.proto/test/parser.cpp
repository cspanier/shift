#include <shift/parser/proto/proto.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <iostream>
#include <sstream>
#include "test.h"

using namespace shift;
using namespace shift::parser;
using namespace shift::parser::proto;

BOOST_AUTO_TEST_CASE(parser_empty_document)
{
  proto::document document;
  test_parser_and_ast(R"()", "test1.pro2");
  test_parser_and_ast(R"(     )", "test2.pro2");
  test_parser_and_ast(R"(
)",
                      "test3.pro2");
  test_parser_and_ast(R"(
  
)",
                      "test4.pro2");
  // BOOST_CHECK(document.empty());
}

BOOST_AUTO_TEST_CASE(parser_basics)
{
  proto::document document;
  if (test_parser_and_ast(document, R"(
namescope test
{
  struct foo
  {
  }

  struct bar<typename T, var_uint_t N> : foo
  {
  }
})",
                          "parser_basics.pro2"))
  {
    const auto* namescope_test = document.root().namescope("test");
    BOOST_CHECK(namescope_test);
    if (namescope_test)
    {
      BOOST_CHECK_EQUAL(namescope_test->identifier, "test");

      const auto* structure_foo = namescope_test->structure("foo");
      BOOST_CHECK(structure_foo);
      if (structure_foo)
      {
        BOOST_CHECK_EQUAL(structure_foo->parent_namescope, namescope_test);
        BOOST_CHECK_EQUAL(structure_foo->identifier, "foo");
        BOOST_CHECK_EQUAL(structure_foo->base, nullptr);
        BOOST_CHECK(structure_foo->template_parameters.empty());
      }
      const auto* structure_bar = namescope_test->structure("bar");
      BOOST_CHECK(structure_bar);
      if (structure_bar)
      {
        BOOST_CHECK_EQUAL(structure_bar->parent_namescope, namescope_test);
        BOOST_CHECK_EQUAL(structure_bar->identifier, "bar");
        BOOST_CHECK_EQUAL(structure_bar->base, structure_foo);
        BOOST_CHECK_EQUAL(structure_bar->template_parameters.size(), 2);

        const auto& first = structure_bar->template_parameters.front();
        const auto& second = structure_bar->template_parameters.back();
        BOOST_CHECK_EQUAL(first.is_typename, true);
        BOOST_CHECK_EQUAL(first.is_variadic, false);
        BOOST_CHECK_EQUAL(first.identifier, "T");
        BOOST_CHECK_EQUAL(second.is_typename, false);
        BOOST_CHECK_EQUAL(second.is_variadic, false);
        BOOST_CHECK_EQUAL(second.identifier, "N");
        /// ToDo: Further investivate template parameters.
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(parser_comments)
{
  proto::document document;
  if (test_parser_and_ast(document, R"(
# some test comment
namescope test
{
  # some multi-line
  # comment about foo
  struct foo
  {
    #no space between # and first word.
    sint32 bar;
  }
})",
                          "parser_comments_1.pro2"))
  {
    const auto* namescope_test = document.root().namescope("test");
    BOOST_CHECK(namescope_test != nullptr);
    if (!namescope_test)
      return;
    BOOST_CHECK_EQUAL(namescope_test->identifier, "test");
    BOOST_CHECK_EQUAL(namescope_test->comments, "some test comment");

    BOOST_CHECK_EQUAL(namescope_test->children.size(), 1);

    const auto* structure_foo = namescope_test->structure("foo");
    BOOST_CHECK(structure_foo != nullptr);
    if (!structure_foo)
      return;
    BOOST_CHECK_EQUAL(structure_foo->identifier, "foo");
    BOOST_CHECK_EQUAL(structure_foo->parent_namescope, namescope_test);
    BOOST_CHECK_EQUAL(structure_foo->base, nullptr);
    BOOST_CHECK_EQUAL(structure_foo->comments,
                      "some multi-line\ncomment about foo");

    const auto* field_bar = structure_foo->field("bar");
    BOOST_CHECK(field_bar != nullptr);
    if (!field_bar)
      return;
    BOOST_CHECK_EQUAL(field_bar->identifier, "bar");
    BOOST_CHECK_EQUAL(field_bar->parent_namescope, namescope_test);
    BOOST_CHECK_EQUAL(field_bar->parent_structure, structure_foo);
    BOOST_CHECK_EQUAL(field_bar->comments,
                      "no space between # and first word.");
  }
}

BOOST_AUTO_TEST_CASE(parser_whitespace)
{
  // Test that missing whitespace behind keywords is rejected.
  test_parser_and_ast(
    "usingfoo=uint32;", "parser_whitespace_1.pro2",
    R"(parser_whitespace_1.pro2(1): error P1100: parse error: Expected whitespace.
usingfoo=uint32;
     ^
)");
  test_parser_and_ast(
    "enumfoo : uint32 {}", "parser_whitespace_2.pro2",
    R"(parser_whitespace_2.pro2(1): error P1100: parse error: Expected whitespace.
enumfoo : uint32 {}
    ^
)");
  test_parser_and_ast(
    "structfoo {}", "parser_whitespace_3.pro2",
    R"(parser_whitespace_3.pro2(1): error P1100: parse error: Expected whitespace.
structfoo {}
      ^
)");
  test_parser_and_ast(
    "namescopefoo {}", "parser_whitespace_4.pro2",
    R"(parser_whitespace_4.pro2(1): error P1100: parse error: Expected whitespace.
namescopefoo {}
         ^
)");

  // Test that line breaks are accepted whitespace characters.
  test_parser_and_ast(
    R"(using
foo=uint32;)",
    "parser_whitespace_5.pro2");
  test_parser_and_ast(
    R"(enum
foo : uint32 {})",
    "parser_whitespace_6.pro2");
  test_parser_and_ast(
    R"(struct
foo {})",
    "parser_whitespace_7.pro2");
  test_parser_and_ast(
    R"(namescope
foo {})",
    "parser_whitespace_8.pro2");
}

BOOST_AUTO_TEST_CASE(validator_symbol_redefinition)
{
  // Defining "namescope foo" twice is valid and the content will be merged.
  test_parser_and_ast(
    R"(namescope test
{
  namescope foo
  {
  }

  namescope foo
  {
  }
})",
    "validator_symbol_redefinition_1.pro2");

  // "namescope foo" is invalid because there is already an "using foo".
  test_parser_and_ast(
    R"(namescope test
{
  using foo = fixed_sint_t<32>;

  namescope foo
  {
  }
})",
    "validator_symbol_redefinition_2.pro2",
    R"(validator_symbol_redefinition_2.pro2(5): error P1001: redefinition of symbol 'foo'
  namescope foo
            ^^^
validator_symbol_redefinition_2.pro2(3): note: see previous definition of 'foo'
  using foo = fixed_sint_t<32>;
        ^^^
)");

  // "namescope foo" is invalid because there is already an "enum foo".
  test_parser_and_ast(
    R"(namescope test
{
  enum foo : fixed_sint<32>
  {
  }

  namescope foo
  {
  }
})",
    "validator_symbol_redefinition_3.pro2",
    R"(validator_symbol_redefinition_3.pro2(7): error P1001: redefinition of symbol 'foo'
  namescope foo
            ^^^
validator_symbol_redefinition_3.pro2(3): note: see previous definition of 'foo'
  enum foo : fixed_sint<32>
       ^^^
)");

  // "namescope foo" is invalid because there is already a "struct foo".
  test_parser_and_ast(
    R"(namescope test
{
  struct foo
  {
  }

  namescope foo
  {
  }
})",
    "validator_symbol_redefinition_4.pro2",
    R"(validator_symbol_redefinition_4.pro2(7): error P1001: redefinition of symbol 'foo'
  namescope foo
            ^^^
validator_symbol_redefinition_4.pro2(3): note: see previous definition of 'foo'
  struct foo
         ^^^
)");

  // Defining a structure with fields having the same identifier.
  test_parser_and_ast(
    R"(struct foo
{
  ufixed8 bar;
  ufixed16 baz;
  ufixed32 baz;
  ufixed64 bar;
})",
    "validator_symbol_redefinition_5.pro2",
    R"(validator_symbol_redefinition_5.pro2(5): error P1001: redefinition of symbol 'baz'
  ufixed32 baz;
           ^^^
validator_symbol_redefinition_5.pro2(4): note: see previous definition of 'baz'
  ufixed16 baz;
           ^^^
validator_symbol_redefinition_5.pro2(6): error P1001: redefinition of symbol 'bar'
  ufixed64 bar;
           ^^^
validator_symbol_redefinition_5.pro2(3): note: see previous definition of 'bar'
  ufixed8 bar;
          ^^^
)");
}

BOOST_AUTO_TEST_CASE(validator_built_in_types)
{
  test_parser_and_ast(
    R"(struct built_in_types
{
  sint8 svi8;
  uint8 uvi8;
  sint16 svi16;
  uint16 uvi16;
  sint32 svi32;
  uint32 uvi32;
  sint64 svi64;
  uint64 uvi64;

  sfixed8 sfi8;
  ufixed8 ufi8;
  sfixed16 sfi16;
  ufixed16 ufi16;
  sfixed32 sfi32;
  ufixed32 ufi32;
  sfixed64 sfi64;
  ufixed64 ufi64;

  bool b;

  char8 c8;
  char16 c16;
  char32 c32;

  float16 half;
  float32 float;
  float64 double;

  string8 s8;
  string16 s16;

  math.vector2f16 v2h;
  math.vector3f16 v3h;
  math.vector4f16 v4h;
  math.vector2f32 v2f;
  math.vector3f32 v3f;
  math.vector4f32 v4f;
  math.vector2f64 v2d;
  math.vector3f64 v3d;
  math.vector4f64 v4d;

  math.matrix22f16 m22h;
  math.matrix33f16 m33h;
  math.matrix44f16 m44h;
  math.matrix22f32 m22f;
  math.matrix33f32 m33f;
  math.matrix44f32 m44f;
  math.matrix22f64 m22d;
  math.matrix33f64 m33d;
  math.matrix44f64 m44d;
})",
    "validator_built_in_types_1.pro2");
}

BOOST_AUTO_TEST_CASE(validator_type_lookup)
{
  test_parser_and_ast(
    R"(namescope foo
{
  struct b {}
  namescope bar
  {
    struct a {}
    struct test1 : a {}
    struct test2 : bar.a {}
    struct test3 : foo.bar.a {}
    struct test4 : b {}
    struct test5 : foo.b {}
  }
})",
    "validator_type_lookup_1.pro2");

  test_parser_and_ast(
    R"(namescope foo
{
  namescope bar
  {
    struct a {}
  }
  struct error : a {}
})",
    "validator_type_lookup_2.pro2",
    R"(validator_type_lookup_2.pro2(7): error P1002: cannot find referenced type 'a'
  struct error : a {}
                 ^
)");

  test_parser_and_ast(
    R"(namescope foo
{
  namescope bar
  {
    struct a {}
  }
  struct error : foo.a {}
})",
    "validator_type_lookup_3.pro2",
    R"(validator_type_lookup_3.pro2(7): error P1002: cannot find referenced type 'foo.a'
  struct error : foo.a {}
                 ^^^^^
)");

  test_parser_and_ast(
    R"(struct foo
{
  int bar;
})",
    "validator_type_lookup_4.pro2",
    R"(validator_type_lookup_4.pro2(3): error P1002: cannot find referenced type 'int'
  int bar;
  ^^^
)");

  test_parser_and_ast(
    R"(using foo = int;)", "validator_type_lookup_5.pro2",
    R"(validator_type_lookup_5.pro2(1): error P1002: cannot find referenced type 'int'
using foo = int;
            ^^^
)");
}
