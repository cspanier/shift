#ifndef SHIFT_RESOURCE_MATERIAL_HPP
#define SHIFT_RESOURCE_MATERIAL_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <shift/math/vector.hpp>
#include "shift/resource/types.hpp"
#include "shift/resource/resource_ptr.hpp"
#include "shift/resource/image.hpp"

namespace shift::resource
{
template <>
struct resource_traits<material>
{
  static constexpr resource_type type_id = resource_type::material;
};

///
struct image_reference
{
  resource_ptr<resource::image> image;
  math::vector2<float> scale = math::vector2<float>(1.0f, 1.0f);
  math::vector2<float> offset = math::vector2<float>(0.0f, 0.0f);
};

enum class sampler_address_mode : std::uint8_t
{
  repeat = 0,
  mirrored_repeat = 1,
  clamp_to_edge = 2,
  clamp_to_border = 3,
  mirror_clamp_to_edge = 4
};

///
struct sampler
{
  /// Equality operator.
  bool operator==(const sampler& other) const;

  /// Inequality operator.
  bool operator!=(const sampler& other) const;

  sampler_address_mode address_mode_u = sampler_address_mode::repeat;
  sampler_address_mode address_mode_v = sampler_address_mode::repeat;
  sampler_address_mode address_mode_w = sampler_address_mode::repeat;
  float max_anisotropy = 1.0f;
  float min_lod = 0.0f;
  float max_lod = 12.0f;
};

using material_parameter =
  std::variant<std::int32_t, math::vector2<std::int32_t>,
               math::vector3<std::int32_t>, math::vector4<std::int32_t>, float,
               math::vector2<float>, math::vector3<float>,
               math::vector4<float>>;

///
class material final : public resource_base
{
public:
  /// Default constructor.
  material() noexcept : resource_base(resource_traits<material>::type_id)
  {
  }

  /// @see resource_base::load.
  void load(resource_id id, boost::iostreams::filtering_istream& stream) final;

  /// @see resource_base::save.
  void save(boost::iostreams::filtering_ostream& stream) const final;

public:
  std::pair<image_reference, sampler> albedo_map;
  std::pair<image_reference, sampler> ambient_occlusion_map;
  std::pair<image_reference, sampler> normal_map;
  std::pair<image_reference, sampler> height_map;
  std::pair<image_reference, sampler> roughness_map;
  std::pair<image_reference, sampler> metalness_map;
  std::pair<image_reference, sampler> specular_map;  /// ToDo: Remove this.

  /// A dictionary of material parameters, which can be overridden in a model.
  std::unordered_map<std::string, material_parameter> material_parameters;

protected:
  /// @see resource_base::hash.
  void hash(crypto::sha256& context) const final;
};
}

#endif
