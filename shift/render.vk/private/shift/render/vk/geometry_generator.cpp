#include "shift/render/vk/geometry_generator.h"
#include <cstring>

namespace shift::render::vk
{
// clang-format off
////         6 -------- 7
////        /|         /|
////       / |        / |
////      4 -------- 5  |
////      |  |       |  |
////  z   |  2 ------|- 3
////  ^  y| /        | /
////  | 7 |/         |/
////  |/  0 -------- 1
////  +----> x
//// clang-format on
//const std::array<math::vector3<float>, 8> cube_generator::cube_corners = {
//  math::vector3<float>{-1.0f, -1.0f, -1.0f /*, 1.0f*/},
//  math::vector3<float>{+1.0f, -1.0f, -1.0f /*, 1.0f*/},
//  math::vector3<float>{-1.0f, +1.0f, -1.0f /*, 1.0f*/},
//  math::vector3<float>{+1.0f, +1.0f, -1.0f /*, 1.0f*/},
//  math::vector3<float>{-1.0f, -1.0f, +1.0f /*, 1.0f*/},
//  math::vector3<float>{+1.0f, -1.0f, +1.0f /*, 1.0f*/},
//  math::vector3<float>{-1.0f, +1.0f, +1.0f /*, 1.0f*/},
//  math::vector3<float>{+1.0f, +1.0f, +1.0f /*, 1.0f*/}};
//
//const std::array<math::vector3<float>, 6> cube_generator::cube_normals = {
//  math::vector3<float>{-1.0f, 0.0f, 0.0f},
//  math::vector3<float>{+1.0f, 0.0f, 0.0f},
//  math::vector3<float>{0.0f, -1.0f, 0.0f},
//  math::vector3<float>{0.0f, +1.0f, 0.0f},
//  math::vector3<float>{0.0f, 0.0f, -1.0f},
//  math::vector3<float>{0.0f, 0.0f, +1.0f}};
//
//// clang-format off
//const std::array<vertex_textured_t, 24> cube_generator::cube_vertices = {
//  // -X side
//  vertex_textured_t{cube_positions[2], cube_normals[0]/*, cube_normals[2]*/, {0.0f, 0.0f}},
//  vertex_textured_t{cube_positions[0], cube_normals[0]/*, cube_normals[2]*/, {1.0f, 0.0f}},
//  vertex_textured_t{cube_positions[4], cube_normals[0]/*, cube_normals[2]*/, {1.0f, 1.0f}},
//  vertex_textured_t{cube_positions[6], cube_normals[0]/*, cube_normals[2]*/, {0.0f, 1.0f}},
//  // +X side
//  vertex_textured_t{cube_positions[1], cube_normals[1]/*, cube_normals[3]*/, {0.0f, 0.0f}},
//  vertex_textured_t{cube_positions[3], cube_normals[1]/*, cube_normals[3]*/, {1.0f, 0.0f}},
//  vertex_textured_t{cube_positions[7], cube_normals[1]/*, cube_normals[3]*/, {1.0f, 1.0f}},
//  vertex_textured_t{cube_positions[5], cube_normals[1]/*, cube_normals[3]*/, {0.0f, 1.0f}},
//  // -Y side
//  vertex_textured_t{cube_positions[0], cube_normals[2]/*, cube_normals[1]*/, {0.0f, 0.0f}},
//  vertex_textured_t{cube_positions[1], cube_normals[2]/*, cube_normals[1]*/, {1.0f, 0.0f}},
//  vertex_textured_t{cube_positions[5], cube_normals[2]/*, cube_normals[1]*/, {1.0f, 1.0f}},
//  vertex_textured_t{cube_positions[4], cube_normals[2]/*, cube_normals[1]*/, {0.0f, 1.0f}},
//  // +Y side
//  vertex_textured_t{cube_positions[3], cube_normals[3]/*, cube_normals[0]*/, {0.0f, 0.0f}},
//  vertex_textured_t{cube_positions[2], cube_normals[3]/*, cube_normals[0]*/, {1.0f, 0.0f}},
//  vertex_textured_t{cube_positions[6], cube_normals[3]/*, cube_normals[0]*/, {1.0f, 1.0f}},
//  vertex_textured_t{cube_positions[7], cube_normals[3]/*, cube_normals[0]*/, {0.0f, 1.0f}},
//  // -Z side
//  vertex_textured_t{cube_positions[1], cube_normals[4]/*, cube_normals[0]*/, {0.0f, 0.0f}},
//  vertex_textured_t{cube_positions[0], cube_normals[4]/*, cube_normals[0]*/, {1.0f, 0.0f}},
//  vertex_textured_t{cube_positions[2], cube_normals[4]/*, cube_normals[0]*/, {1.0f, 1.0f}},
//  vertex_textured_t{cube_positions[3], cube_normals[4]/*, cube_normals[0]*/, {0.0f, 1.0f}},
//  // +Z side
//  vertex_textured_t{cube_positions[6], cube_normals[5]/*, cube_normals[1]*/, {0.0f, 0.0f}},
//  vertex_textured_t{cube_positions[4], cube_normals[5]/*, cube_normals[1]*/, {1.0f, 0.0f}},
//  vertex_textured_t{cube_positions[5], cube_normals[5]/*, cube_normals[1]*/, {1.0f, 1.0f}},
//  vertex_textured_t{cube_positions[7], cube_normals[5]/*, cube_normals[1]*/, {0.0f, 1.0f}}};
//
//const std::array<cube_generator::index_t, 36> cube_generator::cube_indices = {
//   0,  1,  2,
//   0,  2,  3,
//   4,  5,  6,
//   4,  6,  7,
//   8,  9, 10,
//   8, 10, 11,
//  12, 13, 14,
//  12, 14, 15,
//  16, 17, 18,
//  16, 18, 19,
//  20, 21, 22,
//  20, 22, 23};
// clang-format on
//
// std::size_t cube_generator::vertex_count() const
//{
//  return cube_vertices.size();
//}
//
// std::size_t cube_generator::index_count() const
//{
//  return cube_indices.size();
//}
//
// void cube_generator::generate(position_t* positions, normal_t* normals,
//                              texcoord_t* texcoords, index_t* indices)
//{
//  std::memcpy(vertices, cube_vertices.data(), vertices_size());
//  std::memcpy(indices, cube_indices.data(), indices_size());
//}

// clang-format off
//         6 -------- 7
//                   /|
//                  / |
//      4          5  |
//      |             |
//  z   |  2          3
//  ^  y| /
//  | 7 |/
//  |/  0 -------- 1
//  +----> x
const std::array<position_t, 8 * 4> box_generator::box_positions = {{
  position_t{-1.0f, -1.0f, -1.0f},
  position_t{-0.5f, -1.0f, -1.0f},
  position_t{-1.0f, -0.5f, -1.0f},
  position_t{-1.0f, -1.0f, -0.5f},

  position_t{+1.0f, -1.0f, -1.0f},
  position_t{+0.5f, -1.0f, -1.0f},
  position_t{+1.0f, -0.5f, -1.0f},
  position_t{+1.0f, -1.0f, -0.5f},

  position_t{-1.0f, +1.0f, -1.0f},
  position_t{-0.5f, +1.0f, -1.0f},
  position_t{-1.0f, +0.5f, -1.0f},
  position_t{-1.0f, +1.0f, -0.5f},

  position_t{+1.0f, +1.0f, -1.0f},
  position_t{+0.5f, +1.0f, -1.0f},
  position_t{+1.0f, +0.5f, -1.0f},
  position_t{+1.0f, +1.0f, -0.5f},

  position_t{-1.0f, -1.0f, +1.0f},
  position_t{-0.5f, -1.0f, +1.0f},
  position_t{-1.0f, -0.5f, +1.0f},
  position_t{-1.0f, -1.0f, +0.5f},

  position_t{+1.0f, -1.0f, +1.0f},
  position_t{+0.5f, -1.0f, +1.0f},
  position_t{+1.0f, -0.5f, +1.0f},
  position_t{+1.0f, -1.0f, +0.5f},

  position_t{-1.0f, +1.0f, +1.0f},
  position_t{-0.5f, +1.0f, +1.0f},
  position_t{-1.0f, +0.5f, +1.0f},
  position_t{-1.0f, +1.0f, +0.5f},

  position_t{+1.0f, +1.0f, +1.0f},
  position_t{+0.5f, +1.0f, +1.0f},
  position_t{+1.0f, +0.5f, +1.0f},
  position_t{+1.0f, +1.0f, +0.5f}}};

const std::array<index_t, 8 * 3 * 2> box_generator::box_indices = {{
   0,  1,  0,  2,  0,  3,
   4,  5,  4,  6,  4,  7,
   8,  9,  8, 10,  8, 11,
  12, 13, 12, 14, 12, 15,
  16, 17, 16, 18, 16, 19,
  20, 21, 20, 22, 20, 23,
  24, 25, 24, 26, 24, 27,
  28, 29, 28, 30, 28, 31}};
// clang-format on

std::size_t box_generator::vertex_count() const
{
  return box_positions.size();
}

std::size_t box_generator::index_count() const
{
  return box_indices.size();
}

void box_generator::generate(position_t* positions, normal_t* /* normals */,
                             texcoord_t* /* texcoords */, index_t* indices)
{
  if (positions)
  {
    std::memcpy(positions, box_positions.data(),
                box_positions.size() * sizeof(position_t));
  }
  if (indices)
  {
    std::memcpy(indices, box_indices.data(),
                box_indices.size() * sizeof(index_t));
  }
}

// uv_sphere_generator::uv_sphere_generator(
//  math::vector<2, std::uint32_t> subdivision)
//: _subdivision(subdivision)
//{
//}
//
// std::size_t uv_sphere_generator::vertex_count() const
//{
//  return _subdivision.longitude * _subdivision.latitude;
//}
//
// std::size_t uv_sphere_generator::index_count() const
//{
//}
//
// void uv_sphere_generator::generate(position_t* positions, normal_t* normals,
//                                   texcoord_t* texcoords, index_t* indices)
//{
//}
}
