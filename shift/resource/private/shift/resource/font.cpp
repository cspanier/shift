#include "shift/resource/font.hpp"
#include "shift/resource/repository.hpp"
#include <shift/serialization2/all.hpp>
#include <shift/crypto/sha256.hpp>
#include <set>

namespace shift::resource
{
///
serialization2::compact_input_archive<>& operator>>(
  serialization2::compact_input_archive<>& archive, resource::glyph& glyph)
{
  archive >> glyph.solid_sub_mesh_id >> glyph.curved_sub_mesh_id >>
    glyph.width >> glyph.height >> glyph.bearing_x >> glyph.bearing_y >>
    glyph.advance_x >> glyph.advance_y;
  return archive;
}

///
serialization2::compact_output_archive<>& operator<<(
  serialization2::compact_output_archive<>& archive,
  const resource::glyph& glyph)
{
  archive << glyph.solid_sub_mesh_id << glyph.curved_sub_mesh_id << glyph.width
          << glyph.height << glyph.bearing_x << glyph.bearing_y
          << glyph.advance_x << glyph.advance_y;
  return archive;
}

void font::load(resource_id id, boost::iostreams::filtering_istream& stream)
{
  _id = id;
  serialization2::compact_input_archive<> archive{stream};
  archive >> mesh >> ascent >> descent >> line_gap >> glyphs;
}

void font::save(boost::iostreams::filtering_ostream& stream) const
{
  serialization2::compact_output_archive<> archive{stream};
  archive << mesh << ascent << descent << line_gap << glyphs;
}

void font::hash(crypto::sha256& context) const
{
  context << "font" << mesh.id() << ascent << descent << line_gap;

  for (auto& glyph_iter : glyphs)
  {
    const auto& glyph = glyph_iter.second;
    context << static_cast<std::int16_t>(glyph_iter.first)
            << glyph.solid_sub_mesh_id << glyph.curved_sub_mesh_id
            << glyph.width << glyph.height << glyph.bearing_x << glyph.bearing_y
            << glyph.advance_x << glyph.advance_y;
  }
}
}
