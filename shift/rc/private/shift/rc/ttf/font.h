#ifndef SHIFT_RC_TTF_FONT_H
#define SHIFT_RC_TTF_FONT_H

#include <map>
#include <boost/filesystem/path.hpp>
#include <shift/math/interval.h>
#include <shift/serialization2/types.h>

namespace shift::rc::ttf
{
inline constexpr std::uint32_t multi_char_constant(const char constant[4])
{
  return static_cast<std::uint32_t>(constant[0]) << 24 |
         static_cast<std::uint32_t>(constant[1]) << 16 |
         static_cast<std::uint32_t>(constant[2]) << 8 |
         static_cast<std::uint32_t>(constant[3]);
}

enum class table_tag : std::uint32_t
{
  fftm = multi_char_constant("FFTM"),
  gdef = multi_char_constant("GDEF"),
  gpos = multi_char_constant("GPOS"),
  gsub = multi_char_constant("GSUB"),
  os_2 = multi_char_constant("OS/2"),
  cmap = multi_char_constant("cmap"),
  cvt = multi_char_constant("cvt "),
  fpgm = multi_char_constant("fpgm"),
  gasp = multi_char_constant("gasp"),
  glyf = multi_char_constant("glyf"),
  head = multi_char_constant("head"),
  hhea = multi_char_constant("hhea"),
  hmtx = multi_char_constant("hmtx"),
  loca = multi_char_constant("loca"),
  maxp = multi_char_constant("maxp"),
  name = multi_char_constant("name"),
  post = multi_char_constant("post"),
  prep = multi_char_constant("prep")
};

struct table
{
  std::uint32_t checksum;
  std::uint32_t offset;
  std::uint32_t length;
};

struct hhea_table
{
  std::uint16_t version_major;
  std::uint16_t version_minor;
  std::int16_t ascent;
  std::int16_t descent;
  std::int16_t line_gap;
  std::uint16_t advance_width_max;
  std::int16_t min_left_side_bearing;
  std::int16_t min_right_side_bearing;
  std::int16_t x_max_extent;
  std::int16_t caret_slope_rise;
  std::int16_t caret_slope_run;
  std::int16_t caret_offset;
  std::uint16_t reserved1;
  std::uint16_t reserved2;
  std::uint16_t reserved3;
  std::uint16_t reserved4;
  std::int16_t metric_data_format;
  std::uint16_t num_of_long_hor_metrics;
};

struct cmap_segment
{
  std::uint16_t start_code;
  std::uint16_t end_code;
  std::int16_t id_delta;
  std::uint16_t id_range_offset;
};

struct cmap_encoding
{
  std::uint16_t platform_id;
  std::uint16_t encoding_id;
  std::uint32_t offset;
};

struct point
{
  bool on_curve;
  math::vector2<std::int16_t> position;
};

///
struct glyph
{
  bool loaded = false;
  char16_t code = 0;
  std::int16_t contours_count = 0;
  math::rectangle<std::int16_t> bounds;
  std::vector<std::uint16_t> contour_ends;
  std::vector<point> points;
  std::uint16_t advance_x = 0;
  std::uint16_t advance_y = 0;
  std::int16_t bearing_x = 0;
  std::int16_t bearing_y = 0;
};

///
class font
{
public:
  ///
  font();

  ///
  void load(boost::filesystem::path input_filename);

  ///
  std::uint16_t code_to_index(char16_t code) const;

  ///
  const hhea_table& horizontal_header() const
  {
    return _horizontal_header;
  }

  ///
  const std::vector<glyph>& glyphs() const
  {
    return _glyphs;
  }

  ///
  float scale() const
  {
    return 1.0f / _units_per_em;
  }

private:
  using glyphs_t = std::vector<glyph>;

  ///
  void read_offset_tables();

  ///
  std::uint32_t calculate_table_checksum(std::uint32_t offset,
                                         std::uint32_t length);

  ///
  void read_head_table();

  ///
  void read_hhea_table();

  ///
  void read_hmtx_table();

  ///
  void read_glyphs();

  ///
  bool read_glyph(std::uint32_t glyph_index);

  ///
  bool read_simple_glyph(glyph& new_glyph);

  ///
  bool read_compound_glyph(glyph& new_glyph);

  ///
  void read_character_map();

  boost::iostreams::filtering_istream* _stream = nullptr;
  serialization2::compact_input_archive<boost::endian::order::big>* _archive =
    nullptr;
  std::map<table_tag, table> _tables;

  std::uint32_t _scalar_type;
  std::uint16_t _search_range;
  std::uint16_t _entity_selector;
  std::uint16_t _range_shift;

  static constexpr std::uint32_t head_magic_number = 0x5f0f3cf5;

  std::uint16_t _version_major;
  std::uint16_t _version_minor;
  std::uint16_t _font_revision_major;
  std::uint16_t _font_revision_minor;
  std::uint32_t _checksum_adjustment;
  std::uint32_t _magic_number;
  std::uint16_t _flags;
  std::uint16_t _units_per_em;
  std::uint64_t _created;
  std::uint64_t _modified;
  std::int16_t _x_min;
  std::int16_t _y_min;
  std::int16_t _x_max;
  std::int16_t _y_max;
  std::uint16_t _mac_style;
  std::uint16_t _lowest_rec_ppem;
  std::uint16_t _font_direction_hint;
  std::uint16_t _index_to_loc_format;
  std::uint16_t _glyph_data_format;

  hhea_table _horizontal_header;

  glyphs_t _glyphs;
  std::vector<std::uint32_t> _glyph_offsets;
  std::vector<cmap_segment> _cmap_segments;
  std::vector<std::uint16_t> _cmap_glyph_indices;
};
}

#endif
