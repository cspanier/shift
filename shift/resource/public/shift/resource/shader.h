#ifndef SHIFT_RESOURCE_SHADER_H
#define SHIFT_RESOURCE_SHADER_H

#include <vector>
#include "shift/resource/resource_ptr.h"

namespace shift::resource
{
enum class stage_usage : uint32_t
{
  unknown = 0,
  position = 1,
  normal = 2,
  texcoord = 3
};

enum class stage_type : uint32_t
{
  none,
  float16,
  float32,
  float64,
  vec2,
  vec3,
  vec4,
  mat2,
  mat3,
  mat4,
  boolean,
  char8,
  int32,
  uint32,
  int64,
  uint64,
  atomic,
  structure,
  image,
  sampler,
  sampled_image
};

struct stage_binding
{
  stage_usage usage;
  stage_type type;
  std::uint32_t location;
  std::uint32_t component;
};

/// A shader points to binary SPIR-V code blob.
class shader final : public resource_base
{
public:
  static constexpr resource_type static_type = resource_type::shader;

  /// Default constructor.
  shader() noexcept : resource_base(static_type)
  {
  }

  /// @see resource_base::load.
  void load(resource_id id, boost::iostreams::filtering_istream& stream) final;

  /// @see resource_base::save.
  void save(boost::iostreams::filtering_ostream& stream) const final;

public:
  std::vector<std::uint32_t> storage;
  std::vector<stage_binding> stage_inputs;
  std::vector<stage_binding> stage_outputs;

protected:
  /// @see resource_base::hash.
  void hash(crypto::sha256& context) const final;
};
}

#endif
