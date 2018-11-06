#include "shift/proto/grammar.hpp"
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/spirit/include/phoenix.hpp>
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
#include <boost/phoenix/phoenix.hpp>
#include <boost/phoenix/fusion.hpp>
#include <shift/core/boost_restore_warnings.hpp>

namespace shift::proto
{
skipper::skipper() : skipper::base_type(_skipper)
{
  using namespace iso8859_1;
  using qi::lit;
  using qi::eol;

  _skipper %= space | (lit('#') >> *(char_ - eol) >> eol);
}

grammar::grammar() : grammar::base_type(_global_scope)
{
  using namespace phoenix;
  using namespace qi::labels;
  using namespace ascii;
  using qi::lit;
  using qi::lexeme;
  using qi::eps;
  using qi::attr;
  using qi::int_;
  using qi::long_long;
  using qi::ulong_long;
  using qi::bool_;
  using qi::hex;
  using qi::no_skip;

  _sint %=
    lexeme[(lit("0x") >> hex) | (lit('\'') >> char_ >> lit('\'')) | long_long];
  _uint %=
    lexeme[(lit("0x") >> hex) | (lit('\'') >> char_ >> lit('\'')) | ulong_long];
  _namescope_name %= lexeme[(char_("a-z") >> *char_("a-z0-9_"))];
  _typename %= lexeme[char_("A-Za-z") >> *char_("a-zA-Z0-9_")];
  _interface_name %=
    lexeme[char_("Ii") >> char_("A-Za-z") >> *(char_("a-zA-Z0-9"))];
  _type_path %=
    *((_namescope_name | _typename | _interface_name) >> lit('.')) >> _typename;
  _interface_path %= *(_namescope_name >> lit('.')) >> _interface_name;
  _identifier %= lexeme[char_("a-z") >> *char_("a-zA-Z0-9_")];
  _string %= lit('"') >> no_skip[*(char_ - char_('"'))] >> lit('"');

  _bool_type.add("bool", built_in_type::boolean);
  _char_type.add("char8", built_in_type::char8);
  _char_type.add("char16", built_in_type::char16);
  _char_type.add("char32", built_in_type::char32);
  _sint_type.add("int8", built_in_type::int8);
  _sint_type.add("int16", built_in_type::int16);
  _sint_type.add("int32", built_in_type::int32);
  _sint_type.add("int64", built_in_type::int64);
  _uint_type.add("uint8", built_in_type::uint8);
  _uint_type.add("uint16", built_in_type::uint16);
  _uint_type.add("uint32", built_in_type::uint32);
  _uint_type.add("uint64", built_in_type::uint64);
  _uint_type.add("datetime", built_in_type::datetime);
  _float_type.add("float32", built_in_type::float32);
  _float_type.add("float64", built_in_type::float64);
  _string_type.add("string", built_in_type::string);
  _template_type.add("raw_ptr", built_in_type::raw_ptr);
  _template_type.add("unique_ptr", built_in_type::unique_ptr);
  _template_type.add("shared_ptr", built_in_type::shared_ptr);
  _template_type.add("weak_ptr", built_in_type::weak_ptr);
  _template_type.add("group_ptr", built_in_type::group_ptr);
  _template_type.add("tuple", built_in_type::tuple);
  _template_type.add("array", built_in_type::array);
  _template_type.add("list", built_in_type::list);
  _template_type.add("vector", built_in_type::vector);
  _template_type.add("set", built_in_type::set);
  _template_type.add("matrix", built_in_type::matrix);
  _template_type.add("map", built_in_type::map);
  _template_type.add("variant", built_in_type::variant);
  _template_type.add("bitfield", built_in_type::bitfield);

  _int_type %= _sint_type | _uint_type;
  _built_in_type %= _bool_type | _char_type | _int_type | _float_type |
                    _string_type | _template_type;

  _attribute %=
    _identifier > ((lit('=') > (_string | _uint)) |
                   eps[at_c<1>(_val) = static_cast<std::uint64_t>(1)]);
  _attributes %= lit('[') > -(_attribute % lit(',')) > lit(']');
  _type_reference %=
    (_built_in_type | _type_path) >
    -(lit('<') > ((_type_reference | int_) % lit(',')) > lit('>'));

  _alias %= (-_attributes >> lit("using")) > _typename > lit('=') >
            _type_reference > lit(';');

  _enumerator %= _identifier[at_c<0>(_val) = _1] >
                 // If there is an explicit value defined use it.
                 ((lit('=') > _sint[at_c<1>(_val) = _1]) |
                  // Otherwise use the value of argument _r1 instead.
                  eps[at_c<1>(_val) = _r1]);
  _enumeration %= (-_attributes >> lit("enum")) > _typename > lit(':') >
                  _type_reference > lit('{') >
                  // Initialize _a with 0.
                  eps[_a = 0] >
                  // Set _a to the value of the last member + 1.
                  -(_enumerator(_a)[_a = at_c<1>(_1) + 1] % lit(',')) >
                  lit('}');

  _field %= -_attributes >> _type_reference >> _identifier;

  _message %= (-_attributes >> (lit("message") | lit("struct"))) > _typename >
              -(lit("extends") > _type_path) > lit('{') > *(_field > lit(';')) >
              lit('}');
  _interface %= (-_attributes >> lit("interface")) > _interface_name >
                lit('{') > *_message > lit('}');
  _service_type.add("service_client", false);
  _service_type.add("service_provider", true);
  _service %= -_attributes >> _service_type >> lit('<') >> _interface_path >>
              lit(',') >> _interface_path >> lit('>') >> _typename >> lit(';');

  _global_scope %=
    *(_namescope | _alias | _enumeration | _message | _interface | _service);
  _namescope %= (-_attributes >> lit("namescope")) > _namescope_name >
                lit('{') > _global_scope > lit('}');

  BOOST_SPIRIT_DEBUG_NODE(_sint);
  BOOST_SPIRIT_DEBUG_NODE(_uint);
  BOOST_SPIRIT_DEBUG_NODE(_namescope_name);
  BOOST_SPIRIT_DEBUG_NODE(_typename);
  BOOST_SPIRIT_DEBUG_NODE(_interface_name);
  BOOST_SPIRIT_DEBUG_NODE(_type_path);
  BOOST_SPIRIT_DEBUG_NODE(_interface_path);
  BOOST_SPIRIT_DEBUG_NODE(_identifier);
  BOOST_SPIRIT_DEBUG_NODE(_string);
  BOOST_SPIRIT_DEBUG_NODE(_bool_type);
  BOOST_SPIRIT_DEBUG_NODE(_char_type);
  BOOST_SPIRIT_DEBUG_NODE(_sint_type);
  BOOST_SPIRIT_DEBUG_NODE(_uint_type);
  BOOST_SPIRIT_DEBUG_NODE(_float_type);
  BOOST_SPIRIT_DEBUG_NODE(_string_type);
  BOOST_SPIRIT_DEBUG_NODE(_template_type);
  BOOST_SPIRIT_DEBUG_NODE(_int_type);
  BOOST_SPIRIT_DEBUG_NODE(_built_in_type);
  BOOST_SPIRIT_DEBUG_NODE(_attribute);
  BOOST_SPIRIT_DEBUG_NODE(_attributes);
  BOOST_SPIRIT_DEBUG_NODE(_type_reference);
  BOOST_SPIRIT_DEBUG_NODE(_alias);
  BOOST_SPIRIT_DEBUG_NODE(_enumerator);
  BOOST_SPIRIT_DEBUG_NODE(_enumeration);
  BOOST_SPIRIT_DEBUG_NODE(_field);
  BOOST_SPIRIT_DEBUG_NODE(_message);
  BOOST_SPIRIT_DEBUG_NODE(_interface);
  BOOST_SPIRIT_DEBUG_NODE(_service_type);
  BOOST_SPIRIT_DEBUG_NODE(_service);
  BOOST_SPIRIT_DEBUG_NODE(_namescope);

  // Error diagnostic messages which are displayed if one if the rules fail.
  std::map<std::string, std::string> diagnostics;
  diagnostics[_sint.name()] = "Expected signed integer constant.";
  diagnostics[_uint.name()] = "Expected unsigned integer constant.";
  diagnostics[_identifier.name()] =
    "Expected lower camel case formatted identifier.";
  diagnostics[_bool_type.name()] = "Expected boolean type.";
  diagnostics[_char_type.name()] = "Expected character type.";
  diagnostics[_sint_type.name()] = "Expected signed integer.";
  diagnostics[_uint_type.name()] = "Expected unsigned integer.";
  diagnostics[_float_type.name()] = "Expected floating point number.";
  diagnostics[_string_type.name()] = "Expected string type.";
  diagnostics[_template_type.name()] = "Expected container type.";
  diagnostics[_int_type.name()] = "Expected integer constant.";
  diagnostics[_enumerator.name()] = "Non-unique enumerator name.";
  qi::on_error<qi::fail>(_namescope,
                         _error_handler(diagnostics, _1, _2, _3, _4));
}
}

#define ADAPT_MEMBER(TYPE, MEMBER) (decltype(TYPE::MEMBER), MEMBER)
// clang-format off
BOOST_FUSION_ADAPT_STRUCT(shift::proto::type_reference,
  ADAPT_MEMBER(shift::proto::type_reference, name)
  ADAPT_MEMBER(shift::proto::type_reference, arguments))

BOOST_FUSION_ADAPT_STRUCT(shift::proto::namescope,
  ADAPT_MEMBER(shift::proto::namescope, public_attributes)
  ADAPT_MEMBER(shift::proto::namescope, name)
  ADAPT_MEMBER(shift::proto::namescope, static_children))

BOOST_FUSION_ADAPT_STRUCT(shift::proto::alias,
  ADAPT_MEMBER(shift::proto::alias, public_attributes)
  ADAPT_MEMBER(shift::proto::alias, name)
  ADAPT_MEMBER(shift::proto::alias, reference))

BOOST_FUSION_ADAPT_STRUCT(shift::proto::enumerator,
  ADAPT_MEMBER(shift::proto::enumerator, name)
  ADAPT_MEMBER(shift::proto::enumerator, value))

BOOST_FUSION_ADAPT_STRUCT(shift::proto::enumeration,
  ADAPT_MEMBER(shift::proto::enumeration, public_attributes)
  ADAPT_MEMBER(shift::proto::enumeration, name)
  ADAPT_MEMBER(shift::proto::enumeration, base)
  ADAPT_MEMBER(shift::proto::enumeration, members))

BOOST_FUSION_ADAPT_STRUCT(shift::proto::field,
  ADAPT_MEMBER(shift::proto::field, public_attributes)
  ADAPT_MEMBER(shift::proto::field, reference)
  ADAPT_MEMBER(shift::proto::field, name))

BOOST_FUSION_ADAPT_STRUCT(shift::proto::message,
  ADAPT_MEMBER(shift::proto::message, public_attributes)
  ADAPT_MEMBER(shift::proto::message, name)
  ADAPT_MEMBER(shift::proto::message, base_name)
  ADAPT_MEMBER(shift::proto::message, fields))

BOOST_FUSION_ADAPT_STRUCT(shift::proto::interface,
  ADAPT_MEMBER(shift::proto::interface, public_attributes)
  ADAPT_MEMBER(shift::proto::interface, name)
  ADAPT_MEMBER(shift::proto::interface, static_children))

BOOST_FUSION_ADAPT_STRUCT(shift::proto::service,
  ADAPT_MEMBER(shift::proto::service, public_attributes)
  ADAPT_MEMBER(shift::proto::service, is_provider)
  ADAPT_MEMBER(shift::proto::service, service_name)
  ADAPT_MEMBER(shift::proto::service, callback_name)
  ADAPT_MEMBER(shift::proto::service, name))
// clang-format on
#undef ADAPT_MEMBER
