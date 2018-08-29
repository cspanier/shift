#define BOOST_BIND_NO_PLACEHOLDERS

#include "shift/parser/proto/grammar.h"
#include <shift/core/boost_disable_warnings.h>
#include <boost/fusion/include/all.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/tuple.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/preprocessor/arithmetic/dec.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/control/expr_iif.hpp>
#include <boost/preprocessor/control/iif.hpp>
#include <boost/preprocessor/logical/bool.hpp>
#include <boost/preprocessor/repetition/detail/for.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/variadic/elem.hpp>
#include <boost/phoenix.hpp>
#include <shift/core/boost_restore_warnings.h>

namespace shift::parser::proto
{
grammar::grammar(const std::shared_ptr<source_module>& source)
: grammar::base_type(_namescope_content),
  _annotation(annotation_handler<iterator>{source}),
  _error(error_handler<iterator>{source})
{
  using namespace phoenix;
  using namespace qi::labels;
  using encoding::char_;
  using encoding::space;
  using qi::attr;
  using qi::bool_;
  using qi::eol;
  using qi::eps;
  using qi::hex;
  using qi::int_;
  using qi::lexeme;
  using qi::lit;
  using qi::long_long;
  using qi::no_skip;
  using qi::ulong_long;

  _optional_space %= no_skip[-space];
  _whitespace %= no_skip[+space];
  _period %= lit('.');
  _colon %= lit(':');
  _semicolon %= lit(';');
  _comma %= lit(',');
  _equal_sign %= lit('=');
  _bracket_open %= lit('[');
  _bracket_close %= lit(']');
  _curly_bracket_open %= lit('{');
  _curly_bracket_close %= lit('}');
  _angle_bracket_open %= lit('<');
  _angle_bracket_close %= lit('>');
  _single_quote %= lit('\'');
  _quote %= lit('"');

  _sint %=
    lexeme[(lit("0x") >> hex) | (lit('\'') >> char_ >> lit('\'')) | long_long];
  _uint %=
    lexeme[(lit("0x") >> hex) | (lit('\'') >> char_ >> lit('\'')) | ulong_long];
  _string_impl %= _quote >> no_skip[*(char_ - char_('"'))] >> _quote;
  _string %= _string_impl;

  _identifier_impl %= lexeme[char_("a-z") >> *char_("a-z0-9_")];
  _identifier %= _identifier_impl;
  _template_identifier_impl %= lexeme[char_("A-Z") >> *char_("a-zA-Z0-9_")];
  _template_identifier %= _template_identifier_impl;
  _any_identifier %= _identifier | _template_identifier;
  _template_argument %= _type_path | _uint | _sint;
  _type_path_element %=
    _any_identifier > -(_angle_bracket_open > (_template_argument % _comma) >
                        _angle_bracket_close);
  _type_path %= _type_path_element % _period;

  _comment %= lit('#') > _optional_space > no_skip[*(char_ - eol)];

  _attribute %=
    _identifier > ((_equal_sign > (_string | _uint)) |
                   eps[at_c<1>(_val) = static_cast<std::uint64_t>(1)]);
  _attributes %= _bracket_open > -(_attribute % _comma) > _bracket_close;
  _enumerant_reference %= _type_path > _period > _identifier;

  _template_parameter %=
    (lit("typename") > attr(/* type = */ type_path_token{}) >
     attr(/* is_typename = */ true) >
     ((lit("...") > attr(/* is_variadic = */ true)) |
      (_whitespace > attr(/* is_variadic = */ false))) > _template_identifier >
     ((_equal_sign > (_type_path | _uint | _sint)) |
      attr(/* default_value = */ nullptr))) |
    (_type_path > attr(/* is_typename = */ false) >
     ((lit("...") > attr(/* is_variadic = */ true)) |
      (_whitespace > attr(/* is_variadic = */ false))) > _template_identifier >
     ((_equal_sign > (_type_path | _uint | _sint)) |
      attr(/* default_value = */ nullptr)));
  _template_parameters_impl %= _template_parameter % _comma;
  _template_parameters %=
    _angle_bracket_open > _template_parameters_impl > _angle_bracket_close;

  _meta %= *_comment >> -_attributes;

  // alias definitions
  _alias %= (_meta >> lit("using")) > _whitespace > _identifier >
            -_template_parameters > _equal_sign > _type_path > _semicolon;

  // enumeration definitions
  _enumerant_value %= (_equal_sign > (_template_identifier |
                                      _enumerant_reference | _uint | _sint)) |
                      attr(no_value_t{});
  _enumerant %= _identifier > _enumerant_value;
  _enumeration %= (_meta >> lit("enum")) > _whitespace > _identifier >
                  -_template_parameters > _colon > _type_path >
                  _curly_bracket_open > -(_enumerant % _comma) >
                  _curly_bracket_close;

  // structure definitions
  _const_qualifier %= -(lit("const") > _whitespace > attr(true)) | attr(false);
  _field_value %=
    -(_equal_sign > ((lit("nullptr") > attr(nullptr)) | _template_identifier |
                     _enumerant_reference | _uint | _sint)) |
    attr(no_value_t{});
  _field %=
    _meta >> _const_qualifier >> _type_path >> _identifier >> _field_value;
  _structure %= (_meta >> lit("struct")) > _whitespace > _identifier >
                -_template_parameters > -(_colon > _type_path) >
                _curly_bracket_open > *(_field > _semicolon) >
                _curly_bracket_close;

  _namescope_content %= *(_namescope | _alias | _enumeration | _structure);
  _namescope %= (_meta >> lit("namescope")) > _whitespace > _identifier >
                _curly_bracket_open > _namescope_content > _curly_bracket_close;

  BOOST_SPIRIT_DEBUG_NODE(_whitespace);
  BOOST_SPIRIT_DEBUG_NODE(_period);
  BOOST_SPIRIT_DEBUG_NODE(_colon);
  BOOST_SPIRIT_DEBUG_NODE(_semicolon);
  BOOST_SPIRIT_DEBUG_NODE(_comma);
  BOOST_SPIRIT_DEBUG_NODE(_equal_sign);
  BOOST_SPIRIT_DEBUG_NODE(_bracket_open);
  BOOST_SPIRIT_DEBUG_NODE(_bracket_close);
  BOOST_SPIRIT_DEBUG_NODE(_curly_bracket_open);
  BOOST_SPIRIT_DEBUG_NODE(_curly_bracket_close);
  BOOST_SPIRIT_DEBUG_NODE(_angle_bracket_open);
  BOOST_SPIRIT_DEBUG_NODE(_angle_bracket_close);
  BOOST_SPIRIT_DEBUG_NODE(_single_quote);
  BOOST_SPIRIT_DEBUG_NODE(_quote);
  BOOST_SPIRIT_DEBUG_NODE(_sint);
  BOOST_SPIRIT_DEBUG_NODE(_uint);
  BOOST_SPIRIT_DEBUG_NODE(_string);
  BOOST_SPIRIT_DEBUG_NODE(_identifier);
  BOOST_SPIRIT_DEBUG_NODE(_template_identifier);
  BOOST_SPIRIT_DEBUG_NODE(_any_identifier);
  BOOST_SPIRIT_DEBUG_NODE(_template_argument);
  BOOST_SPIRIT_DEBUG_NODE(_type_path_element);
  BOOST_SPIRIT_DEBUG_NODE(_type_path);
  BOOST_SPIRIT_DEBUG_NODE(_comment);
  BOOST_SPIRIT_DEBUG_NODE(_attribute);
  BOOST_SPIRIT_DEBUG_NODE(_attributes);
  BOOST_SPIRIT_DEBUG_NODE(_enumerant_reference);
  BOOST_SPIRIT_DEBUG_NODE(_template_parameter);
  BOOST_SPIRIT_DEBUG_NODE(_template_parameters_impl);
  BOOST_SPIRIT_DEBUG_NODE(_template_parameters);
  BOOST_SPIRIT_DEBUG_NODE(_alias);
  BOOST_SPIRIT_DEBUG_NODE(_enumerant_value);
  BOOST_SPIRIT_DEBUG_NODE(_enumerant);
  BOOST_SPIRIT_DEBUG_NODE(_enumeration);
  BOOST_SPIRIT_DEBUG_NODE(_const_qualifier);
  BOOST_SPIRIT_DEBUG_NODE(_field_value);
  BOOST_SPIRIT_DEBUG_NODE(_field);
  BOOST_SPIRIT_DEBUG_NODE(_structure);
  BOOST_SPIRIT_DEBUG_NODE(_namescope_content);
  BOOST_SPIRIT_DEBUG_NODE(_namescope);

  // Error diagnostic messages which are displayed if one of the rules fail.
  // Note: The error code is calculated from the message's vector position.
  // Inserting a new entry in between breaks all test cases which check for
  // specific error codes.
  _diagnostics.emplace_back(_whitespace.name(), "Expected whitespace.");
  _diagnostics.emplace_back(_period.name(), "Expected period ('.').");
  _diagnostics.emplace_back(_colon.name(), "Expected colon (':').");
  _diagnostics.emplace_back(_semicolon.name(), "Expected semicolon (';').");
  _diagnostics.emplace_back(_comma.name(), "Expected comma (',').");
  _diagnostics.emplace_back(_equal_sign.name(),
                            "Expected equality sign ('=').");
  _diagnostics.emplace_back(_bracket_open.name(),
                            "Expected opening bracket ('[').");
  _diagnostics.emplace_back(_bracket_close.name(),
                            "Expected closing bracket (']').");
  _diagnostics.emplace_back(_curly_bracket_open.name(),
                            "Expected opening curly bracket ('{').");
  _diagnostics.emplace_back(_curly_bracket_close.name(),
                            "Expected closing curly bracket ('}').");
  _diagnostics.emplace_back(_angle_bracket_open.name(),
                            "Expected opening angle bracket ('<').");
  _diagnostics.emplace_back(_angle_bracket_close.name(),
                            "Expected closing angle bracket ('>').");
  _diagnostics.emplace_back(_single_quote.name(),
                            "Expected single quotation mark (''').");
  _diagnostics.emplace_back(_quote.name(), "Expected quotation mark ('\"').");

  _diagnostics.emplace_back(_sint.name(), "Expected signed integer constant.");
  _diagnostics.emplace_back(_uint.name(),
                            "Expected unsigned integer constant.");
  _diagnostics.emplace_back(_identifier.name(), "Expected identifier.");
  _diagnostics.emplace_back(_template_identifier.name(),
                            "Expected template identifier.");
  _diagnostics.emplace_back(_any_identifier.name(), "Expected identifier.");
  _diagnostics.emplace_back(_type_path_element.name(), "Expected type path.");
  _diagnostics.emplace_back(_type_path.name(), "Expected type path.");
  _diagnostics.emplace_back(_template_parameters_impl.name(),
                            "Expected list of template parameters.");
  _diagnostics.emplace_back(_enumerant.name(), "Non-unique enumerant name.");

  // Assign on_success handlers that capture the location of parsed tokens.
  auto set_annotation_info = _annotation(_val, _1, _2, _3);
  qi::on_success(_identifier, set_annotation_info);
  qi::on_success(_template_identifier, set_annotation_info);
  qi::on_success(_type_path_element, set_annotation_info);
  qi::on_success(_type_path, set_annotation_info);
  qi::on_success(_comment, set_annotation_info);
  qi::on_success(_attribute, set_annotation_info);
  qi::on_success(_attributes, set_annotation_info);
  qi::on_success(_template_parameter, set_annotation_info);
  qi::on_success(_template_parameters_impl, set_annotation_info);
  qi::on_success(_alias, set_annotation_info);
  qi::on_success(_enumerant, set_annotation_info);
  qi::on_success(_enumeration, set_annotation_info);
  qi::on_success(_field, set_annotation_info);
  qi::on_success(_structure, set_annotation_info);
  qi::on_success(_namescope, set_annotation_info);

  // Assign on_error handlers that print diagnostic information about an error.
  auto handle_error = _error(_diagnostics, _1, _2, _3, _4);
  qi::on_error<qi::fail>(_sint, handle_error);
  qi::on_error<qi::fail>(_uint, handle_error);
  qi::on_error<qi::fail>(_string, handle_error);
  qi::on_error<qi::fail>(_identifier, handle_error);
  qi::on_error<qi::fail>(_template_identifier, handle_error);
  qi::on_error<qi::fail>(_any_identifier, handle_error);
  qi::on_error<qi::fail>(_template_argument, handle_error);
  qi::on_error<qi::fail>(_type_path_element, handle_error);
  qi::on_error<qi::fail>(_type_path, handle_error);
  qi::on_error<qi::fail>(_comment, handle_error);
  qi::on_error<qi::fail>(_attribute, handle_error);
  qi::on_error<qi::fail>(_attributes, handle_error);
  qi::on_error<qi::fail>(_const_qualifier, handle_error);
  qi::on_error<qi::fail>(_enumerant_reference, handle_error);
  qi::on_error<qi::fail>(_template_parameter, handle_error);
  qi::on_error<qi::fail>(_template_parameters_impl, handle_error);
  qi::on_error<qi::fail>(_template_parameters, handle_error);
  qi::on_error<qi::fail>(_alias, handle_error);
  qi::on_error<qi::fail>(_enumerant, handle_error);
  qi::on_error<qi::fail>(_enumeration, handle_error);
  qi::on_error<qi::fail>(_field_value, handle_error);
  qi::on_error<qi::fail>(_field, handle_error);
  qi::on_error<qi::fail>(_structure, handle_error);
  qi::on_error<qi::fail>(_namescope_content, handle_error);
  qi::on_error<qi::fail>(_namescope, handle_error);
}
}

using namespace shift::parser::proto;

#define ADAPT_MEMBER(TYPE, MEMBER) (decltype(TYPE::MEMBER), MEMBER)
// clang-format off
BOOST_FUSION_ADAPT_STRUCT(string_token,
  ADAPT_MEMBER(string_token, value))

BOOST_FUSION_ADAPT_STRUCT(type_path_element_token,
  ADAPT_MEMBER(type_path_element_token, name)
  ADAPT_MEMBER(type_path_element_token, template_arguments))

BOOST_FUSION_ADAPT_STRUCT(type_attribute_token,
  ADAPT_MEMBER(type_attribute_token, identifier)
  ADAPT_MEMBER(type_attribute_token, value))

BOOST_FUSION_ADAPT_STRUCT(meta_token,
  ADAPT_MEMBER(meta_token, comments)
  ADAPT_MEMBER(meta_token, attributes))

BOOST_FUSION_ADAPT_STRUCT(enumerant_reference_token,
  ADAPT_MEMBER(enumerant_reference_token, name)
  ADAPT_MEMBER(enumerant_reference_token, enumerant))

BOOST_FUSION_ADAPT_STRUCT(template_parameter_token,
  ADAPT_MEMBER(template_parameter_token, type)
  ADAPT_MEMBER(template_parameter_token, is_typename)
  ADAPT_MEMBER(template_parameter_token, is_variadic)
  ADAPT_MEMBER(template_parameter_token, identifier)
  ADAPT_MEMBER(template_parameter_token, default_value))

BOOST_FUSION_ADAPT_STRUCT(alias_token,
  ADAPT_MEMBER(alias_token, meta)
  ADAPT_MEMBER(alias_token, identifier)
  ADAPT_MEMBER(alias_token, template_parameters)
  ADAPT_MEMBER(alias_token, reference))

BOOST_FUSION_ADAPT_STRUCT(enumerant_token,
  ADAPT_MEMBER(enumerant_token, identifier)
  ADAPT_MEMBER(enumerant_token, value))

BOOST_FUSION_ADAPT_STRUCT(enumeration_token,
  ADAPT_MEMBER(enumeration_token, meta)
  ADAPT_MEMBER(enumeration_token, identifier)
  ADAPT_MEMBER(enumeration_token, template_parameters)
  ADAPT_MEMBER(enumeration_token, base)
  ADAPT_MEMBER(enumeration_token, enumerants))

BOOST_FUSION_ADAPT_STRUCT(field_token,
  ADAPT_MEMBER(field_token, meta)
  ADAPT_MEMBER(field_token, is_const)
  ADAPT_MEMBER(field_token, reference)
  ADAPT_MEMBER(field_token, identifier)
  ADAPT_MEMBER(field_token, default_value))

BOOST_FUSION_ADAPT_STRUCT(structure_token,
  ADAPT_MEMBER(structure_token, meta)
  ADAPT_MEMBER(structure_token, identifier)
  ADAPT_MEMBER(structure_token, template_parameters)
  ADAPT_MEMBER(structure_token, base)
  ADAPT_MEMBER(structure_token, fields))

BOOST_FUSION_ADAPT_STRUCT(namescope_token,
  ADAPT_MEMBER(namescope_token, meta)
  ADAPT_MEMBER(namescope_token, identifier)
  ADAPT_MEMBER(namescope_token, children))

//BOOST_FUSION_ADAPT_STRUCT(interface,
//  ADAPT_MEMBER(interface, attributes)
//  ADAPT_MEMBER(interface, identifier)
//  ADAPT_MEMBER(interface, static_children))
//
//BOOST_FUSION_ADAPT_STRUCT(service,
//  ADAPT_MEMBER(service, attributes)
//  ADAPT_MEMBER(service, is_provider)
//  ADAPT_MEMBER(service, service_name)
//  ADAPT_MEMBER(service, callback_name)
//  ADAPT_MEMBER(service, identifier))
// clang-format on
#undef ADAPT_MEMBER
