#ifndef SHIFT_RC_OPTIMIZER_MESH_FILTER_HPP
#define SHIFT_RC_OPTIMIZER_MESH_FILTER_HPP

#include <shift/core/exception.hpp>
#include <shift/resource_db/repository.hpp>
#include "shift/rc/types.hpp"

namespace shift::resource_db
{
class mesh;
}

namespace shift::rc
{
///
bool optimize_mesh(const job_description& job);

///
bool optimize_mesh(resource_db::mesh& mesh);

///
std::vector<std::uint8_t> resolve_vertex_indices(
  const std::vector<std::uint8_t>& vertex_buffer, std::size_t vertex_size,
  const std::vector<std::uint8_t>& indices, std::size_t index_size);

///
std::pair<std::vector<std::uint32_t>, std::size_t> generate_index_buffer(
  std::vector<std::uint8_t>& vertex_buffer, std::size_t vertex_size);

///
void compress_vertex_buffer(std::vector<std::uint8_t>& vertex_buffer,
                            std::size_t vertex_size,
                            std::size_t unique_vertex_count,
                            const std::vector<std::uint32_t>& indices);
}

#endif
