#include "shift/resource_db/material.hpp"
#include <shift/serialization2/all.hpp>
#include <shift/math/serialization2.hpp>
#include <shift/math/utility.hpp>
#include <shift/crypto/sha256.hpp>

namespace shift::resource_db
{
bool sampler::operator==(const sampler& other) const
{
  return address_mode_u == other.address_mode_u &&
         address_mode_v == other.address_mode_v &&
         address_mode_w == other.address_mode_w &&
         math::almost_equal(max_anisotropy, other.max_anisotropy) &&
         math::almost_equal(min_lod, other.min_lod) &&
         math::almost_equal(max_lod, other.max_lod);
}

bool sampler::operator!=(const sampler& other) const
{
  return !(*this == other);
}

crypto::sha256& operator<<(crypto::sha256& context,
                           const image_reference& value)
{
  context << value.image.id() << value.scale << value.offset;
  return context;
}

serialization2::compact_input_archive<>& operator>>(
  serialization2::compact_input_archive<>& archive, image_reference& value)
{
  archive >> value.image >> value.scale >> value.offset;
  return archive;
}

serialization2::compact_output_archive<>& operator<<(
  serialization2::compact_output_archive<>& archive,
  const image_reference& value)
{
  archive << value.image << value.scale << value.offset;
  return archive;
}

crypto::sha256& operator<<(crypto::sha256& context, const sampler& value)
{
  context << core::underlying_type_cast(value.address_mode_u)
          << core::underlying_type_cast(value.address_mode_v)
          << core::underlying_type_cast(value.address_mode_w)
          << value.max_anisotropy << value.min_lod << value.max_lod;
  return context;
}

serialization2::compact_input_archive<>& operator>>(
  serialization2::compact_input_archive<>& archive, sampler& value)
{
  archive >> value.address_mode_u >> value.address_mode_v >>
    value.address_mode_w >> value.max_anisotropy >> value.min_lod >>
    value.max_lod;
  return archive;
}

serialization2::compact_output_archive<>& operator<<(
  serialization2::compact_output_archive<>& archive, const sampler& value)
{
  archive << value.address_mode_u << value.address_mode_v
          << value.address_mode_w << value.max_anisotropy << value.min_lod
          << value.max_lod;
  return archive;
}

void material::load(resource_id id, boost::iostreams::filtering_istream& stream)
{
  _id = id;
  serialization2::compact_input_archive<> archive{stream};
  archive >> albedo_map >> ambient_occlusion_map >> normal_map >> height_map >>
    roughness_map >> metalness_map >> specular_map >> material_parameters;
}

void material::save(boost::iostreams::filtering_ostream& stream) const
{
  serialization2::compact_output_archive<> archive{stream};
  archive << albedo_map << ambient_occlusion_map << normal_map << height_map
          << roughness_map << metalness_map << specular_map
          << material_parameters;
}

void material::hash(crypto::sha256& context) const
{
  context << albedo_map.first << albedo_map.second
          << ambient_occlusion_map.first << ambient_occlusion_map.second
          << normal_map.first << normal_map.second << height_map.first
          << height_map.second << roughness_map.first << roughness_map.second
          << metalness_map.first << metalness_map.second << specular_map.first
          << specular_map.second;
  for (const auto& parameter : material_parameters)
    context << parameter.first << parameter.second;
}
}
