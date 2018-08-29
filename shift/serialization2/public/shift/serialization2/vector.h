#ifndef SHIFT_SERIALIZATION2_VECTOR_H
#define SHIFT_SERIALIZATION2_VECTOR_H

#include <vector>
#include "shift/serialization2/types.h"

namespace shift::serialization2
{
///
template <boost::endian::order Order, typename U>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::vector<U>& vector)
{
  begin_vector begin;
  archive >> begin;
  vector.resize(begin.length);
  for (auto& element : vector)
    archive >> element;
  archive >> end_vector{};
  return archive;
}

///
template <boost::endian::order Order, typename U>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::vector<U>& vector)
{
  archive << begin_vector{vector.size()};
  for (const auto& element : vector)
    archive << element;
  archive << end_vector{};
  return archive;
}

/// Special case for std::vector<bool>.
template <boost::endian::order Order>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::vector<bool>& vector)
{
  begin_vector begin;
  archive >> begin;
  vector.resize(begin.length);
  for (std::size_t i = 0; i < vector.size(); ++i)
  {
    bool element;
    archive >> element;
    vector[i] = element;
  }
  archive >> end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::vector<bool>& vector)
{
  archive << begin_vector{vector.size()};
  for (const auto element : vector)
    archive << element;
  archive << end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::vector<char>& vector)
{
  begin_vector begin;
  archive >> begin;
  vector.resize(begin.length);
  archive.read(vector.data(), vector.size() * sizeof(char));
  archive >> end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::vector<char>& vector)
{
  archive << begin_vector{vector.size()};
  archive.write(reinterpret_cast<const char*>(vector.data()),
                vector.size() * sizeof(char));
  archive << end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::vector<char16_t>& vector)
{
  begin_vector begin;
  archive >> begin;
  vector.resize(begin.length);
  archive.read(reinterpret_cast<char*>(vector.data()),
               vector.size() * sizeof(char16_t));
  archive >> end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::vector<char16_t>& vector)
{
  archive << begin_vector{vector.size()};
  archive.write(reinterpret_cast<const char*>(vector.data()),
                vector.size() * sizeof(char16_t));
  archive << end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::vector<char32_t>& vector)
{
  begin_vector begin;
  archive >> begin;
  vector.resize(begin.length);

  archive.read(reinterpret_cast<char*>(vector.data()),
               vector.size() * sizeof(char32_t));
  archive >> end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::vector<char32_t>& vector)
{
  archive << begin_vector{vector.size()};
  archive.write(reinterpret_cast<const char*>(vector.data()),
                vector.size() * sizeof(char32_t));
  archive << end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::vector<std::uint8_t>& vector)
{
  begin_vector begin;
  archive >> begin;
  vector.resize(begin.length);

  archive.read(reinterpret_cast<char*>(vector.data()),
               vector.size() * sizeof(std::uint8_t));
  archive >> end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive,
  const std::vector<std::uint8_t>& vector)
{
  archive << begin_vector{vector.size()};
  archive.write(reinterpret_cast<const char*>(vector.data()),
                vector.size() * sizeof(std::uint8_t));
  archive << end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::vector<std::int8_t>& vector)
{
  begin_vector begin;
  archive >> begin;
  vector.resize(begin.length);

  archive.read(reinterpret_cast<char*>(vector.data()),
               vector.size() * sizeof(std::int8_t));
  archive >> end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive,
  const std::vector<std::int8_t>& vector)
{
  archive << begin_vector{vector.size()};
  archive.write(reinterpret_cast<const char*>(vector.data()),
                vector.size() * sizeof(std::int8_t));
  archive << end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::vector<std::uint16_t>& vector)
{
  begin_vector begin;
  archive >> begin;
  vector.resize(begin.length);

  archive.read(reinterpret_cast<char*>(vector.data()),
               vector.size() * sizeof(std::uint16_t));
  archive >> end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive,
  const std::vector<std::uint16_t>& vector)
{
  archive << begin_vector{vector.size()};
  archive.write(reinterpret_cast<const char*>(vector.data()),
                vector.size() * sizeof(std::uint16_t));
  archive << end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::vector<std::int16_t>& vector)
{
  begin_vector begin;
  archive >> begin;
  vector.resize(begin.length);

  archive.read(reinterpret_cast<char*>(vector.data()),
               vector.size() * sizeof(std::int16_t));
  archive >> end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive,
  const std::vector<std::int16_t>& vector)
{
  archive << begin_vector{vector.size()};
  archive.write(reinterpret_cast<const char*>(vector.data()),
                vector.size() * sizeof(std::int16_t));
  archive << end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::vector<std::uint32_t>& vector)
{
  begin_vector begin;
  archive >> begin;
  vector.resize(begin.length);

  archive.read(reinterpret_cast<char*>(vector.data()),
               vector.size() * sizeof(std::uint32_t));
  archive >> end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive,
  const std::vector<std::uint32_t>& vector)
{
  archive << begin_vector{vector.size()};
  archive.write(reinterpret_cast<const char*>(vector.data()),
                vector.size() * sizeof(std::uint32_t));
  archive << end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::vector<std::int32_t>& vector)
{
  begin_vector begin;
  archive >> begin;
  vector.resize(begin.length);

  archive.read(reinterpret_cast<char*>(vector.data()),
               vector.size() * sizeof(std::int32_t));
  archive >> end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive,
  const std::vector<std::int32_t>& vector)
{
  archive << begin_vector{vector.size()};
  archive.write(reinterpret_cast<const char*>(vector.data()),
                vector.size() * sizeof(std::int32_t));
  archive << end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::vector<std::uint64_t>& vector)
{
  begin_vector begin;
  archive >> begin;
  vector.resize(begin.length);

  archive.read(reinterpret_cast<char*>(vector.data()),
               vector.size() * sizeof(std::uint64_t));
  archive >> end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive,
  const std::vector<std::uint64_t>& vector)
{
  archive << begin_vector{vector.size()};
  archive.write(reinterpret_cast<const char*>(vector.data()),
                vector.size() * sizeof(std::uint64_t));
  archive << end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::vector<std::int64_t>& vector)
{
  begin_vector begin;
  archive >> begin;
  vector.resize(begin.length);

  archive.read(reinterpret_cast<char*>(vector.data()),
               vector.size() * sizeof(std::int64_t));
  archive >> end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive,
  const std::vector<std::int64_t>& vector)
{
  archive << begin_vector{vector.size()};
  archive.write(reinterpret_cast<const char*>(vector.data()),
                vector.size() * sizeof(std::int64_t));
  archive << end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::vector<float>& vector)
{
  begin_vector begin;
  archive >> begin;
  vector.resize(begin.length);

  archive.read(reinterpret_cast<char*>(vector.data()),
               vector.size() * sizeof(float));
  archive >> end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::vector<float>& vector)
{
  archive << begin_vector{vector.size()};
  archive.write(reinterpret_cast<const char*>(vector.data()),
                vector.size() * sizeof(float));
  archive << end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::vector<double>& vector)
{
  begin_vector begin;
  archive >> begin;
  vector.resize(begin.length);

  archive.read(reinterpret_cast<char*>(vector.data()),
               vector.size() * sizeof(double));
  archive >> end_vector{};
  return archive;
}

///
template <boost::endian::order Order>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::vector<double>& vector)
{
  archive << begin_vector{vector.size()};
  archive.write(reinterpret_cast<const char*>(vector.data()),
                vector.size() * sizeof(double));
  archive << end_vector{};
  return archive;
}
}

#endif
