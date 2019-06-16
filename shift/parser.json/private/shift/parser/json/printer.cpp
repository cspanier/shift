#include "shift/parser/json/json.hpp"
#include <iomanip>

#if defined(SHIFT_PLATFORM_WINDOWS)
#define br "\r\n"
#else
#define br "\n"
#endif
#define br2 br br

namespace shift::parser::json
{
///
int escape_strings_index()
{
  static int index = std::ios_base::xalloc();
  return index;
}

///
void indent_character(std::ios_base& stream, bool enable)
{
  stream.iword(escape_strings_index()) = (enable ? 1 : -1);
}

///
core::implementation::io_manipulator<bool> escape_strings(bool enable)
{
  return core::implementation::io_manipulator<bool>(&indent_character, enable);
}

///
struct printer
{
  using return_type = void;

  /// Prints an object.
  void operator()(const json::object& object);

  ///
  void operator()(const std::nullptr_t& value);

  ///
  void operator()(const bool& value);

  ///
  void operator()(const double& value);

  ///
  void operator()(const std::int64_t& value);

  ///
  void operator()(const std::string& value);

  ///
  void operator()(const array& values);

  std::ostream& stream;
};

void printer::operator()(const json::object& object)
{
  stream << "{" << core::inc_indent;
  bool first = true;
  for (const auto& key_value_pair : object)
  {
    if (first)
      first = false;
    else
      stream << ',';
    stream << br << core::indent;
    (*this)(key_value_pair.first);
    stream << " : ";
    json::visit(*this, key_value_pair.second);
  }
  stream << core::dec_indent;
  if (!object.empty())
    stream << br << core::indent;
  stream << "}";
}

void printer::operator()(const std::nullptr_t& /*null_value*/)
{
  stream << "null";
}

void printer::operator()(const bool& value)
{
  stream << (value ? "true" : "false");
}

void printer::operator()(const double& value)
{
  stream << std::setprecision(std::numeric_limits<double>::digits10) << value;
}

void printer::operator()(const std::int64_t& value)
{
  stream << value;
}

void printer::operator()(const std::string& value)
{
  if (stream.iword(escape_strings_index()) != 0)
  {
    stream << '"';
    for (const auto c : value)
    {
      switch (c)
      {
      case '"':
        stream << "\\\"";
        break;
      case '\\':
        stream << "\\\\";
        break;
      // case '/':
      //  stream << "\\/";
      //  break;
      case '\b':
        stream << "\\b";
        break;
      case '\f':
        stream << "\\f";
        break;
      case '\n':
        stream << "\\n";
        break;
      case '\r':
        stream << "\\r";
        break;
      case '\t':
        stream << "\\t";
        break;
      default:
        if (c < 32 || c > 126)
          stream << "\\u" << std::hex << std::setfill('0') << std::setw(4)
                 << static_cast<int>(c) << std::dec;
        else
          stream << c;
        break;
      }
    }
    stream << '"';
  }
  else
    stream << '"' << value << '"';
}

void printer::operator()(const json::array& values)
{
  stream << core::inc_indent << '[';
  bool first = true;
  for (const auto& value : values)
  {
    if (first)
      first = false;
    else
      stream << ",";
    stream << br << core::indent;
    json::visit(*this, value);
  }
  stream << core::dec_indent;
  if (!values.empty())
    stream << br << core::indent;
  stream << ']';
}

std::ostream& operator<<(std::ostream& stream, const json::object& object)
{
  if (stream.iword(escape_strings_index()) == 0)
    stream << escape_strings(true);
  printer{stream}(object);
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const json::array& values)
{
  printer{stream}(values);
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const json::value& value)
{
  json::visit(printer{stream}, value);
  return stream;
}
}
