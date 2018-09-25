#ifndef SHIFT_RESOURCE_FONT_H
#define SHIFT_RESOURCE_FONT_H

#include <unordered_map>
#include "shift/resource/resource_ptr.h"
#include "shift/resource/mesh.h"

namespace shift::resource
{
/// A glyph contains all information about rendering a single character.
struct glyph
{
  /// The id of the sub-mesh for the inner solid geometry, or 0 if there is
  /// none for this glyph.
  std::uint32_t solid_sub_mesh_id = 0;

  /// The id of the sub-mesh for the outer curved geometry, or 0 if there is
  /// none for this glyph.
  std::uint32_t curved_sub_mesh_id = 0;

  float width;
  float height;
  float bearing_x;
  float bearing_y;
  float advance_x;
  float advance_y;
};

/// A font is a collection of glyphs, sharing a single mesh for quick
/// rendering.
class font final : public resource_base
{
public:
  static constexpr resource_type static_type = resource_type::font;

  /// Default constructor.
  font() noexcept : resource_base(static_type)
  {
  }

  /// @see resource_base::load.
  void load(resource_id id, boost::iostreams::filtering_istream& stream) final;

  /// @see resource_base::save.
  void save(boost::iostreams::filtering_ostream& stream) const final;

public:
  /// A mesh containing all font geometry data. Each glyph uses up to two
  /// sub-meshes of this mesh.
  resource_ptr<resource::mesh> mesh;

  ///
  float ascent = 0.0f;

  ///
  float descent = 0.0f;

  ///
  float line_gap = 0.0f;

  /// Dictionary of all glyphs stored in this font.
  std::map<char16_t, glyph> glyphs;

protected:
  /// @see resource_base::hash.
  void hash(crypto::sha256& context) const final;
};
}

#endif
