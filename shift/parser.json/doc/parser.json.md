# shift.parser.json - Yet another JSON Parser

This tiny library implements a high performance JSON parser.

## Overview

The parser is based on [Boost Spirit](https://www.boost.org/doc/libs/1_68_0/libs/spirit/doc/html/index.html) Qi. This allows for an incredibly easy and robust grammar definition:

```C++
  _null.add("null", nullptr);
  _unescape_map.add(R"(\")", '\"');
  _unescape_map.add(R"(\\)", '\\');
  _unescape_map.add(R"(\/)", '/');
  _unescape_map.add(R"(\b)", '\b');
  _unescape_map.add(R"(\f)", '\f');
  _unescape_map.add(R"(\n)", '\n');
  _unescape_map.add(R"(\r)", '\r');
  _unescape_map.add(R"(\t)", '\t');
  _string %=
    '"' > no_skip[*(_unescape_map | ("\\u" >> qi::hex) | (qi::char_ - '"'))] >
    '"';
  _value %= _null | bool_ | double_ | _string | _object | _array;
  _array %= '[' > -(_value % ',') > ']';
  _key_value %= _string > ':' > _value;
  _object %= '{' > -(_key_value % ',') > '}';
```

The parsed document is stored in a data structure consisting mostly of standard [std::string](https://en.cppreference.com/w/cpp/string/basic_string), [std::vector](https://en.cppreference.com/w/cpp/container/vector), and [std::map](https://en.cppreference.com/w/cpp/container/map) types. The only exception is the value type, which is a simple wrapper of [std::variant](https://en.cppreference.com/w/cpp/utility/variant) and necessary only to break circular dependencies between all types.

## Comments

The skipper rule is defined as `_rule %= space | (lit("#") > *(char_ - eol));`, which allows for (non-standard) comments in JSON documents using the hash character ('#').

## Usage

The primary interface is `std::istream& operator>>(std::istream& stream, json::value& value);`, which parses the content of any istream and stores the JSON document in a root value. The actual value behind the variant type can be retrieved with either `json::get` or `json::get_if`, which work equivalent to [`std::get`](https://en.cppreference.com/w/cpp/utility/variant/get) and [`std::get_if`](https://en.cppreference.com/w/cpp/utility/variant/get_if) on std::variant types. To check for `null` use `std::nullptr_t`.

## Example

```C++
  std::istringstream document(R"({"k1" : false, "k2" : null})");
  json::value root;
  document >> root;
  auto& root_object = json::get<json::object>(root);
  auto k1 = json::get<bool>(root_object, "k1");
  auto k2 = json::get<std::nullptr_t>(root_object, "k2");
```
