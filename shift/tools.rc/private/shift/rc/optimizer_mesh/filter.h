#ifndef SHIFT_RC_OPTIMIZER_MESH_FILTER_H
#define SHIFT_RC_OPTIMIZER_MESH_FILTER_H

#include <shift/core/exception.h>
#include <shift/resource/repository.h>
#include "shift/rc/types.h"

namespace shift::resource
{
class mesh;
}

namespace shift::rc
{
///
bool optimize_mesh(const job_description& job);

///
bool optimize_mesh(resource::mesh& mesh);

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
