#ifndef SHIFT_CORE_HEXDUMP_H
#define SHIFT_CORE_HEXDUMP_H

#include <array>
#include <iostream>
#include <iomanip>
#include "shift/core/core.h"

namespace shift::core
{
///
template <std::size_t BytesPerRow, typename TContainer>
struct hex_dump_t
{
  using value_type = typename TContainer::value_type;

  static_assert(sizeof(value_type) == 1,
                "Using shift::core::hexdump with containers of types larger "
                "than one byte is not supported, yet");
  static_assert(std::is_convertible<value_type, std::uint8_t>::value,
                "Using shift::core::hexdump with containers of types which "
                "are not convertible to uint8_t is not supported.");

  /// Constructor.
  inline hex_dump_t(const TContainer& data) : data(data)
  {
  }

  const TContainer& data;
};

template <std::size_t BytesPerRow, typename TContainer>
inline hex_dump_t<BytesPerRow, TContainer> hex_dump(const TContainer& data)
{
  return hex_dump_t<BytesPerRow, TContainer>(data);
}

template <std::size_t BytesPerRow, typename TContainer>
std::ostream& operator<<(std::ostream& stream,
                         const hex_dump_t<BytesPerRow, TContainer>& dump)
{
  std::array<char, BytesPerRow> row;

  auto write_row = [&](std::size_t count) {
    if (count == 0)
      return;
    for (std::size_t i = 0; i < count; ++i)
    {
      stream << std::setw(2)
             << static_cast<std::uint32_t>(static_cast<std::uint8_t>(row[i]))
             << " ";
    }
    for (std::size_t i = count; i < BytesPerRow; ++i)
      stream << "   ";
    for (std::size_t i = 0; i < count; ++i)
    {
      auto c = row[i];
      stream << ((c >= ' ' && c <= '~') ? c : '.');
    }
    stream << std::endl;
  };

  auto last_fill_char = stream.fill('0');
  stream << std::hex << std::setfill('0');
  std::size_t column = 0;
  for (auto c : dump.data)
  {
    row[column] = c;
    if (++column >= BytesPerRow)
    {
      write_row(column);
      column = 0;
    }
  }
  write_row(column);
  stream.fill(last_fill_char);
  return stream;
}
}

#endif
