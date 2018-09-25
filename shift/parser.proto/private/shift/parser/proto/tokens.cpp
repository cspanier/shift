#include "shift/parser/proto/tokens.h"

namespace shift::parser::proto
{
std::ostream& operator<<(std::ostream& stream, const string_token& value)
{
  stream << value.value;
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const type_path_token& value)
{
  bool first_element = true;
  for (const auto& element : value)
  {
    if (first_element)
      first_element = false;
    else
      stream << ".";
    stream << element.name.value;
    if (!element.template_arguments.empty())
    {
      stream << "<";
      bool first_argument = true;
      for ([[maybe_unused]] auto& argument : element.template_arguments)
      {
        if (first_argument)
          first_argument = false;
        else
          stream << ", ";
        stream << "ToDo" /* argument*/;
      }
      stream << ">";
    }
  }
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const namescope_token& value)
{
  stream << "namescope " << value.identifier << " {}";
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const alias_token& value)
{
  stream << "alias " << value.identifier << " = ...";
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const enumeration_token& value)
{
  stream << "enum " << value.identifier << " {}";
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const structure_token& value)
{
  stream << "struct " << value.identifier << " {}";
  return stream;
}
}
