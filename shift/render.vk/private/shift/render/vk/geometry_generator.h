#ifndef SHIFT_RENDER_VK_GEOMETRY_GENERATOR_H
#define SHIFT_RENDER_VK_GEOMETRY_GENERATOR_H

#include <shift/math/vector.h>

namespace shift::render::vk
{
using position_t = math::vector3<float>;
using normal_t = math::vector3<float>;
using texcoord_t = math::vector2<float>;
using index_t = std::uint32_t;

/////
// class cube_generator : public geometry_generator
//{
// public:
//  ///
//  virtual std::size_t vertex_count() const override;
//
//  ///
//  virtual std::size_t index_count() const override;
//
//  ///
//  virtual void generate(position_t* positions, normal_t* normals,
//                        texcoord_t* texcoords, index_t* indices) override;
//
// private:
//  static const std::array<position_t, 8> cube_positions;
//  static const std::array<normal_t, 6> cube_normals;
//  static const std::array<vertex_textured_t, 4 * 6> cube_vertices;
//  static const std::array<index_t, 2 * 3 * 6> cube_indices;
//};

///
class box_generator
{
public:
  ///
  std::size_t vertex_count() const;

  ///
  std::size_t index_count() const;

  ///
  void generate(position_t* positions, normal_t* /* normals */,
                texcoord_t* /* texcoords */, index_t* indices);

private:
  static const std::array<position_t, 8 * 4> box_positions;
  static const std::array<index_t, 8 * 3 * 2> box_indices;
};

/////
// class uv_sphere_generator
//{
// public:
//  uv_sphere_generator(math::vector<2, std::uint32_t> subdivision);
//
//  ///
//  std::size_t vertex_count() const;
//
//  ///
//  std::size_t index_count() const;
//
//  ///
//  void generate(position_t* positions, normal_t* normals, texcoord_t*
//  texcoords,
//                index_t* indices);
//
// private:
//  math::vector<2, std::uint32_t> _subdivision;
//};
}

#endif
