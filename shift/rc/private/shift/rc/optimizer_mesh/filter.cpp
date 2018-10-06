#include "shift/rc/optimizer_mesh/filter.h"
#include <shift/resource/mesh.h>
#include <shift/log/log.h>
#include <shift/math/vector.h>
#include <shift/core/hash_table.h>
#include <boost/filesystem.hpp>
#include <boost/endian/conversion.hpp>
#include <queue>
#include <memory>

namespace std
{
/// Implementation of the 64bit Murmur hash algorithm to hash vertex data of
/// arbitrary size.
template <>
struct hash<const void*>
{
  std::size_t operator()(const void* vertex) const
  {
    constexpr auto m = static_cast<std::size_t>(0xc6a4a7935bd1e995ULL);
    constexpr int r = 47;

    std::size_t remaining_size = vertex_size;
    std::size_t h = vertex_size;

    // Mix 4 bytes at a time into the hash
    const auto* data = reinterpret_cast<const std::uint8_t*>(vertex);

    while (remaining_size >= sizeof(std::size_t))
    {
      auto k = *reinterpret_cast<const std::size_t*>(data);

      k *= m;
      k ^= k >> r;
      k *= m;

      h *= m;
      h ^= k;

      data += sizeof(std::size_t);
      remaining_size -= sizeof(std::size_t);
    }

    // Handle the last few bytes of the input array
    switch (remaining_size)
    {
    case 7:
      h ^= static_cast<std::size_t>(data[6]) << 48;
      [[fallthrough]];
    case 6:
      h ^= static_cast<std::size_t>(data[5]) << 40;
      [[fallthrough]];
    case 5:
      h ^= static_cast<std::size_t>(data[4]) << 32;
      [[fallthrough]];
    case 4:
      h ^= *reinterpret_cast<const std::uint32_t*>(&data[0]);
      break;
    case 3:
      h ^= static_cast<std::size_t>(data[2]) << 16;
      [[fallthrough]];
    case 2:
      h ^= *reinterpret_cast<const std::uint16_t*>(&data[0]);
      break;
    case 1:
      h ^= static_cast<std::size_t>(data[0]);
    };
    h *= m;

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
  }

  std::size_t vertex_size;
};

/// An overload of std::equal_to to compare arbitrary vertices.
template <>
struct equal_to<const void*>
{
  using first_argument_type = const void*;
  using second_argument_type = const void*;
  using result_type = bool;

  bool operator()(const first_argument_type lhs, const second_argument_type rhs)
  {
    return std::memcmp(lhs, rhs, vertex_size) == 0;
  }

  std::size_t vertex_size;
};
}

namespace shift::rc
{
namespace fs = boost::filesystem;

bool optimize_mesh(const job_description& /*job*/)
{
  // auto input_file_path = job.input_base_path / job.input_file_path;
  // if (!fs::exists(input_file_path) ||
  // !fs::is_regular_file(input_file_path))
  //{
  //  log::error() << "Cannot find input file " << input_file_path << ".";
  //  return false;
  //}

  // std::ifstream file;
  // file.open(input_file_path.generic_string(),
  //          std::ios_base::in | std::ios_base::binary);
  // if (!file.is_open())
  //{
  //  /// ToDo: Throw exception.
  //  return false;
  //}

  // std::vector<std::uint32_t> indices;
  // indices.resize(1000);  // total_indices
  // std::size_t unique_vertex_count;
  // unique_vertex_count =
  //  generate_index_buffer(indices, nullptr, sizeof(vertex));
  // generate_vertex_buffer(indices, vertices.data(), indices.size(),
  //                       sizeof(vertex));

  //// Store resource into repository.
  // resource::repository::singleton_instance().save(*mesh,

  return true;
}

template <typename Index>
std::vector<std::uint8_t> resolve_vertex_indices(resource::mesh& mesh,
                                                 /* Index* indices, */
                                                 std::size_t index_count)
{
  std::size_t vertex_size = 0;
  for (auto& attribute : mesh.vertex_attributes)
  {
    vertex_size += resource::vertex_attribute_size(attribute.component_type,
                                                   attribute.data_type);
  }
  const auto& index_buffer = mesh.index_buffer_view.buffer->storage;

  std::vector<std::uint8_t> destination;
  destination.resize(index_count * vertex_size);
  for (std::size_t i = 0; i < index_count; ++i)
  {
    for (auto& attribute : mesh.vertex_attributes)
    {
      const auto& vertex_buffer = attribute.vertex_buffer_view.buffer->storage;
      std::memcpy(
        destination.data() + i * attribute.stride + attribute.offset,
        vertex_buffer.data() +
          *reinterpret_cast<const Index*>(&index_buffer[i * sizeof(Index)]) *
            attribute.stride +
          attribute.offset,
        attribute.size);
    }
  }
  return destination;
}

bool optimize_mesh(resource::mesh& mesh)
{
  if (mesh.sub_meshes.size() != 1)
    return true;

  for (auto& attribute : mesh.vertex_attributes)
  {
    if (!attribute.vertex_buffer_view.buffer)
      return false;
  }
  std::vector<std::uint8_t> vertex_buffer;

  if (!mesh.index_buffer_view.buffer)
    return false;

  std::size_t index_count = 0;
  for (auto sub_mesh : mesh.sub_meshes)
  {
    switch (sub_mesh.topology)
    {
    case resource::primitive_topology::triangle_list:
      index_count = sub_mesh.index_count / 3;
      break;

    case resource::primitive_topology::triangle_strip:
      /// ToDo: Find and subtract empty triangles.
      BOOST_ASSERT(false);  // Not yet implemented.
      return false;

    case resource::primitive_topology::triangle_fan:
      BOOST_ASSERT(false);  // Not yet implemented.
      return false;

    default:
      BOOST_ASSERT(false);  // Unsupported.
      return false;
    }
  }

  switch (mesh.index_data_type)
  {
  case resource::vertex_index_data_type::uint8:
    resolve_vertex_indices<std::uint8_t>(
      mesh, /*mesh.index_buffer_view.buffer->storage.data(),*/ index_count);
    break;

  default:
    // Ignore other cases for now.
    break;
  }

  // resolve_vertex_indices(mesh.
  return true;
}

std::pair<std::vector<std::uint32_t>, std::size_t> generate_index_buffer(
  std::vector<std::uint8_t>& vertex_buffer, std::size_t vertex_size)
{
  std::size_t vertex_count = vertex_buffer.size() / vertex_size;
  std::vector<std::uint32_t> indices;
  indices.resize(vertex_count);

  std::hash<const void*> hasher{vertex_size};
  std::equal_to<const void*> compare{vertex_size};
  core::hash_table<const void*, std::size_t> table(vertex_count, hasher,
                                                   compare);

  std::size_t next_vertex = 0;
  for (std::size_t i = 0; i < vertex_count; ++i)
  {
    const void* vertex = vertex_buffer.data() + i * vertex_size;
    bool unique;
    std::size_t* value;
    std::tie(unique, value) = table[vertex];
    if (unique)
      *value = next_vertex++;
    indices[i] = *value;
  }

  return std::make_pair(std::move(indices), next_vertex);
}

void compress_vertex_buffer(std::vector<std::uint8_t>& vertex_buffer,
                            std::size_t vertex_size,
                            std::size_t unique_vertex_count,
                            const std::vector<std::uint32_t>& indices)
{
  std::uint32_t next_vertex = 0;
  for (std::size_t i = 0; i < unique_vertex_count; ++i)
  {
    BOOST_ASSERT(indices[i] <= next_vertex);
    if (indices[i] == next_vertex)
    {
      if (next_vertex != i)
      {
        std::memcpy(vertex_buffer.data() + next_vertex * vertex_size,
                    vertex_buffer.data() + i * vertex_size, vertex_size);
      }
      ++next_vertex;
    }
  }
}
}
