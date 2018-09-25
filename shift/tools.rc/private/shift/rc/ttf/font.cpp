#include "shift/rc/ttf/font.h"
#include <shift/serialization2/all.h>
#include <shift/log/log.h>
#include <shift/math/line.h>
#include <shift/math/intersection.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <queue>
#include <memory>

namespace shift::rc::ttf
{
namespace fs = boost::filesystem;

font::font() = default;

void font::load(boost::filesystem::path input_filename)
{
  if (!fs::exists(input_filename) || !fs::is_regular_file(input_filename))
  {
    /// ToDo: Throw exception.
    log::error() << "Cannot open input file " << input_filename << ".";
    return;
  }

  std::ifstream font_file;
  font_file.open(input_filename.generic_string(),
                 std::ios::in | std::ios_base::binary);
  if (!font_file.is_open())
  {
    /// ToDo: Throw exception.
    log::error() << "Cannot open input file " << input_filename << ".";
    return;
  }
  font_file.unsetf(std::ios_base::skipws);
  std::string font_content;
  std::copy(std::istream_iterator<char>(font_file),
            std::istream_iterator<char>(), std::back_inserter(font_content));
  font_file.close();

  boost::iostreams::filtering_istream stream;
  stream.push(
    boost::iostreams::array_source(font_content.data(), font_content.size()));
  serialization2::compact_input_archive<boost::endian::order::big> archive(
    stream);

  _stream = &stream;
  _archive = &archive;

  read_offset_tables();
  read_head_table();
  read_glyphs();
  read_hhea_table();
  read_character_map();

  _stream = nullptr;
  _archive = nullptr;
};

void font::read_offset_tables()
{
  _tables.clear();

  std::uint16_t num_tables;
  *_archive >> _scalar_type >> num_tables >> _search_range >>
    _entity_selector >> _range_shift;

  for (std::uint16_t i = 0; i < num_tables; ++i)
  {
    table_tag tag;
    table table;
    *_archive >> tag >> table.checksum >> table.offset >> table.length;

    if (tag != table_tag::head)
    {
      if (calculate_table_checksum(table.offset, table.length) !=
          table.checksum)
      {
        /// ToDo: Throw exception.
        log::error() << "Wrong table checksum.";
        return;
      }
    }

    _tables.insert(std::make_pair(tag, std::move(table)));
  }
}

std::uint32_t font::calculate_table_checksum(std::uint32_t offset,
                                             std::uint32_t length)
{
  auto old_offset = _stream->tellg();
  _stream->seekg(offset);

  std::uint32_t sum = 0;
  for (auto num_dwords = (length + 3) / 4; num_dwords > 0; --num_dwords)
  {
    std::uint32_t dword;
    *_archive >> dword;
    sum += dword;
  }

  _stream->seekg(old_offset);
  return sum;
}

void font::read_head_table()
{
  auto head_table_iter = _tables.find(table_tag::head);
  if (head_table_iter == _tables.end())
  {
    /// ToDo: Throw exception.
    log::error() << "Cannot locate table 'head'.";
    return;
  }
  auto& head_table = head_table_iter->second;

  _stream->seekg(head_table.offset);
  *_archive >> _version_major >> _version_minor >> _font_revision_major >>
    _font_revision_minor >> _checksum_adjustment >> _magic_number >> _flags >>
    _units_per_em >> _created >> _modified >> _x_min >> _y_min >> _x_max >>
    _y_max >> _mac_style >> _lowest_rec_ppem >> _font_direction_hint >>
    _index_to_loc_format >> _glyph_data_format;
  BOOST_ASSERT(_magic_number == head_magic_number);
  if (_magic_number != head_magic_number)
  {
    /// ToDo: Throw exception.
    log::error() << "wrong magic number in table 'head'.";
    return;
  }
}

void font::read_hhea_table()
{
  auto hhea_table_iter = _tables.find(table_tag::hhea);
  if (hhea_table_iter == _tables.end())
  {
    /// ToDo: Throw exception.
    log::error() << "Cannot locate table 'hhea'.";
    return;
  }
  auto& hhea_table = hhea_table_iter->second;

  _stream->seekg(hhea_table.offset);
  *_archive >> _horizontal_header.version_major >>
    _horizontal_header.version_minor >> _horizontal_header.ascent >>
    _horizontal_header.descent >> _horizontal_header.line_gap >>
    _horizontal_header.advance_width_max >>
    _horizontal_header.min_left_side_bearing >>
    _horizontal_header.min_right_side_bearing >>
    _horizontal_header.x_max_extent >> _horizontal_header.caret_slope_rise >>
    _horizontal_header.caret_slope_run >> _horizontal_header.caret_offset >>
    _horizontal_header.reserved1 >> _horizontal_header.reserved2 >>
    _horizontal_header.reserved3 >> _horizontal_header.reserved4 >>
    _horizontal_header.metric_data_format >>
    _horizontal_header.num_of_long_hor_metrics;

  if (_horizontal_header.version_major != 1 ||
      _horizontal_header.version_minor != 0)
  {
    /// ToDo: Throw exception.
    log::error() << "Unknown version of table 'hhea'.";
    return;
  }

  read_hmtx_table();
}

void font::read_hmtx_table()
{
  auto hmtx_table_iter = _tables.find(table_tag::hmtx);
  if (hmtx_table_iter == _tables.end())
  {
    /// ToDo: Throw exception.
    log::error() << "Cannot locate table 'hmtx'.";
    return;
  }
  auto& hmtx_table = hmtx_table_iter->second;

  if (_glyphs.size() < _horizontal_header.num_of_long_hor_metrics)
  {
    /// ToDo: Throw exception.
    log::error() << "There are fewer glyphs than entries in table 'hmtx'.";
    return;
  }

  _stream->seekg(hmtx_table.offset);
  std::uint16_t advance_x = 0;
  std::int16_t bearing_x;
  for (auto i = 0; i < _horizontal_header.num_of_long_hor_metrics; ++i)
  {
    *_archive >> advance_x >> bearing_x;
    _glyphs[i].advance_x = advance_x;
    _glyphs[i].bearing_x = bearing_x;
  }
  for (auto i = _horizontal_header.num_of_long_hor_metrics; i < _glyphs.size();
       ++i)
  {
    *_archive >> bearing_x;
    _glyphs[i].advance_x = advance_x;
    _glyphs[i].bearing_x = bearing_x;
  }
}

void font::read_glyphs()
{
  auto loca_table_iter = _tables.find(table_tag::loca);
  if (loca_table_iter == _tables.end())
  {
    /// ToDo: Throw exception.
    log::error() << "Cannot locate table 'loca'.";
    return;
  }
  auto& loca_table = loca_table_iter->second;

  _stream->seekg(loca_table.offset);
  // Select between two available offset data formats.
  if (_index_to_loc_format == 1)
  {
    _glyph_offsets.resize(loca_table.length / sizeof(std::uint32_t));
    for (auto& glyph_offset : _glyph_offsets)
      *_archive >> glyph_offset;
  }
  else
  {
    _glyph_offsets.resize(loca_table.length / sizeof(std::uint16_t));
    for (auto& glyph_offset : _glyph_offsets)
    {
      std::uint16_t small_offset;
      *_archive >> small_offset;
      glyph_offset = small_offset * 2;
    }
  }

  _glyphs.resize(_glyph_offsets.size());
  for (std::uint32_t glyph_index = 0; glyph_index < _glyph_offsets.size() - 1;
       ++glyph_index)
  {
    if (!read_glyph(glyph_index))
      return;
  }
}

bool font::read_glyph(std::uint32_t glyph_index)
{
  auto glyf_table_iter = _tables.find(table_tag::glyf);
  if (glyf_table_iter == _tables.end())
  {
    /// ToDo: Throw exception.
    log::error() << "Cannot locate table 'glyf'.";
    return false;
  }
  auto& glyf_table = glyf_table_iter->second;

  auto& new_glyph = _glyphs[glyph_index];
  if (new_glyph.loaded)
    return true;
  else
    new_glyph.loaded = true;

  // If the offset of this glyph is equal to the next one then it doesn't have
  // an outline.
  if (_glyph_offsets[glyph_index] == _glyph_offsets[glyph_index + 1])
    return true;

  _stream->seekg(_glyph_offsets[glyph_index] + glyf_table.offset);
  *_archive >> new_glyph.contours_count >> new_glyph.bounds.min.x >>
    new_glyph.bounds.min.y >> new_glyph.bounds.max.x >> new_glyph.bounds.max.y;

  if (new_glyph.contours_count < -1)
  {
    /// ToDo: Throw exception.
    log::error() << "Illegal number of contours";
    return false;
  }

  if (new_glyph.contours_count >= 0 && read_simple_glyph(new_glyph))
    return true;
  else if (new_glyph.contours_count == -1 && read_compound_glyph(new_glyph))
    return true;
  return false;
}

bool font::read_simple_glyph(glyph& new_glyph)
{
  enum class point_flag : std::uint8_t
  {
    on_curve = 1 << 0,
    x_is_byte = 1 << 1,
    y_is_byte = 1 << 2,
    repeat = 1 << 3,
    x_delta = 1 << 4,
    y_delta = 1 << 5
  };

  using point_flags = core::bit_field<point_flag>;

  //  glyph.type = "simple";

  new_glyph.contour_ends.reserve(new_glyph.contours_count);
  std::uint16_t points_count = 0;
  for (auto i = 0; i < new_glyph.contours_count; ++i)
  {
    std::uint16_t contour_end;
    *_archive >> contour_end;
    points_count = std::max<std::uint16_t>(points_count, contour_end + 1);
    new_glyph.contour_ends.push_back(contour_end);
  }

  if (new_glyph.contours_count == 0)
    return false;

  // Skip over intructions.
  std::uint16_t instructions_count;
  *_archive >> instructions_count;
  _stream->seekg(instructions_count, std::ios_base::cur);

  std::vector<point_flags> flags;
  for (auto i = 0; i != points_count; ++i)
  {
    point_flags flag{0};
    *_archive >> flag.data();
    flags.push_back(flag);
    BOOST_ASSERT(new_glyph.points.size() < points_count);
    new_glyph.points.push_back({flag & point_flag::on_curve,
                                math::make_vector_from<std::int16_t>(0, 0)});

    if (flag & point_flag::repeat)
    {
      std::uint8_t repeat_count;
      *_archive >> repeat_count;
      if (repeat_count == 0)
        log::warning() << "repeat count must be larger than zero.";
      i += repeat_count;
      while (repeat_count-- > 0)
      {
        flags.push_back(flag);
        BOOST_ASSERT(new_glyph.points.size() < points_count);
        new_glyph.points.push_back(
          {flag & point_flag::on_curve,
           math::make_vector_from<std::int16_t>(0, 0)});
      }
    }
  }

  BOOST_ASSERT(new_glyph.points.size() == points_count);

  // Read point coordinates.
  auto is_byte_flag = point_flag::x_is_byte;
  auto delta_flag = point_flag::x_delta;
  for (auto component = 0u; component < 2; ++component,
            is_byte_flag = point_flag::y_is_byte,
            delta_flag = point_flag::y_delta)
  {
    std::int16_t position = 0;
    for (auto i = 0u; i < points_count; ++i)
    {
      auto flag = flags[i];
      if (flag & is_byte_flag)
      {
        std::uint8_t offset;
        *_archive >> offset;
        if (flag & delta_flag)
          position += offset;
        else
          position -= offset;
      }
      else
      {
        if (~flag & delta_flag)
        {
          std::int16_t offset;
          *_archive >> offset;
          position += offset;
        }
      }
      new_glyph.points[i].position(component) = position;
    }
  }

  return true;
}

bool font::read_compound_glyph(glyph& new_glyph)
{
  enum class component_flag : std::uint16_t
  {
    arg1_and_2_are_words = 1 << 0,
    args_are_xy_values = 1 << 1,
    round_xy_to_grid = 1 << 2,
    we_have_a_scale = 1 << 3,
    more_components = 1 << 5,
    we_have_an_x_and_y_scale = 1 << 6,
    we_have_a_two_by_two = 1 << 7,
    we_have_instructions = 1 << 8,
    use_my_metrics = 1 << 9,
    overlap_compound = 1 << 10
  };
  using component_flags = core::bit_field<component_flag>;

  new_glyph.contours_count = 0;

  component_flags flags{0};
  std::uint16_t glyph_index;
  do
  {
    *_archive >> flags >> glyph_index;

    /// ToDo: Make use of dx and dy.
    // float dx;
    // float dy;
    if (flags & component_flag::arg1_and_2_are_words)
    {
      std::uint16_t dx_temp;
      std::uint16_t dy_temp;
      *_archive >> dx_temp >> dy_temp;
      // dx = dx_temp;
      // dy = dy_temp;
    }
    else
    {
      std::uint8_t dx_temp;
      std::uint8_t dy_temp;
      *_archive >> dx_temp >> dy_temp;
      // dx = dx_temp;
      // dy = dy_temp;
    }
    /// ToDo: Do something with dx and dy!

    if (~flags & component_flag::args_are_xy_values)
    {
      log::error() << "Still unsupported";
      return false;
    }

    bool has_transform = false;
    math::matrix22<std::int16_t> transform;
    if (flags & component_flag::we_have_a_scale)
    {
      *_archive >> transform(0, 0);
      transform(1, 1) = transform(0, 0);
      has_transform = true;
    }
    else if (flags & component_flag::we_have_an_x_and_y_scale)
    {
      *_archive >> transform(0, 0) >> transform(1, 1);
      has_transform = true;
    }
    else if (flags & component_flag::we_have_a_two_by_two)
    {
      *_archive >> transform(0, 0) >> transform(1, 0) >> transform(0, 1) >>
        transform(1, 1);
      has_transform = true;
    }

    if (glyph_index >= _glyphs.size())
    {
      /// ToDo: Throw exception.
      return false;
    }
    if (!_glyphs[glyph_index].loaded)
    {
      auto stream_position = _stream->tellg();
      if (!read_glyph(glyph_index))
      {
        /// ToDo: Throw exception.
        return false;
      }
      _stream->seekg(stream_position);
    }

    const auto& component_glyph = _glyphs[glyph_index];
    new_glyph.contours_count += component_glyph.contours_count;
    for (auto contour_end : component_glyph.contour_ends)
    {
      new_glyph.contour_ends.push_back(
        static_cast<std::uint16_t>(new_glyph.points.size() + contour_end));
    }
    for (const auto& new_point : component_glyph.points)
    {
      if (has_transform)
      {
      }
      new_glyph.points.push_back(new_point);
    }
  } while (flags & component_flag::more_components);

  if (flags & component_flag::we_have_instructions)
  {
    std::uint16_t num_instructions;
    *_archive >> num_instructions;
    std::uint8_t instruction;
    for (auto i = num_instructions; i > 0; --i)
      *_archive >> instruction;
  }

  /// ToDo: Compound glyphs are not supported, yet.
  // followed by n component glyph parts.
  // Each component glyph parts consists of
  //   a flag entry,
  //   two offset entries and
  //   from one to four transformation entries.
  return true;
}

void font::read_character_map()
{
  auto cmap_table_iter = _tables.find(table_tag::cmap);
  if (cmap_table_iter == _tables.end())
  {
    /// ToDo: Throw exception.
    log::error() << "Cannot locate table 'cmap'.";
    return;
  }
  auto& cmap_table = cmap_table_iter->second;

  _stream->seekg(cmap_table.offset);

  std::uint16_t version;
  std::uint16_t encodings_count;
  *_archive >> version >> encodings_count;

  if (version != 0)
  {
    log::error() << "Unknown version of table 'cmap'.";
    return;
  }

  std::vector<cmap_encoding> cmap;
  cmap.resize(encodings_count);
  for (auto& encoding : cmap)
  {
    *_archive >> encoding.platform_id >> encoding.encoding_id >>
      encoding.offset;
  }
  bool found_subtable = false;
  std::uint16_t length;
  for (const auto& encoding : cmap)
  {
    if (encoding.platform_id != 3 || encoding.encoding_id != 1)
      continue;
    _stream->seekg(cmap_table.offset + encoding.offset);
    std::uint16_t format;
    *_archive >> format;
    if (format != 4)
      continue;
    std::uint16_t language;
    std::uint16_t segment_count_x2;
    std::uint16_t search_range;
    std::uint16_t entry_selector;
    std::uint16_t range_shift;
    std::uint16_t reserved_pad;

    *_archive >> length >> language >> segment_count_x2 >> search_range >>
      entry_selector >> range_shift;
    _cmap_segments.resize(segment_count_x2 / 2);
    for (auto& segment : _cmap_segments)
      *_archive >> segment.end_code;
    *_archive >> reserved_pad;
    for (auto& segment : _cmap_segments)
      *_archive >> segment.start_code;
    for (auto& segment : _cmap_segments)
      *_archive >> segment.id_delta;
    for (auto& segment : _cmap_segments)
      *_archive >> segment.id_range_offset;

    _cmap_glyph_indices.resize(
      (cmap_table.length + cmap_table.offset - _stream->tellg()) / 2);
    for (auto& glyph_index : _cmap_glyph_indices)
      *_archive >> glyph_index;

    found_subtable = true;
    break;
  }
  if (!found_subtable)
  {
    log::error() << "Could not find a required subtable in 'cmap'.";
    return;
  }

  // Mark all referenced glyphs.
  for (auto i = 0u; i < _cmap_segments.size(); ++i)
  {
    if (_cmap_segments[i].start_code == 0xffff)
      break;
    if (_cmap_segments[i].id_range_offset == 0)
    {
      for (std::uint16_t character_code = _cmap_segments[i].start_code;
           character_code <= _cmap_segments[i].end_code; ++character_code)
      {
        auto glyph_index = static_cast<std::uint16_t>(
          character_code + _cmap_segments[i].id_delta);
        if (_glyphs[glyph_index].code == 0)
          _glyphs[glyph_index].code = static_cast<char16_t>(character_code);
      }
    }
    else
    {
      for (std::uint32_t character_code = _cmap_segments[i].start_code;
           character_code <= _cmap_segments[i].end_code; ++character_code)
      {
        auto lookup_index = _cmap_segments[i].id_range_offset / 2 -
                            (_cmap_segments.size() - i) +
                            (character_code - _cmap_segments[i].start_code);
        auto glyph_index = _cmap_glyph_indices[lookup_index];
        if (_glyphs[glyph_index].code == 0)
          _glyphs[glyph_index].code = static_cast<char16_t>(character_code);
      }
    }
  }
}

std::uint16_t font::code_to_index(char16_t code) const
{
  std::uint16_t character_code = code;

  for (auto i = 0u; i < _cmap_segments.size(); ++i)
  {
    if (_cmap_segments[i].end_code < character_code)
      continue;
    if (_cmap_segments[i].start_code > character_code)
      return 0;

    if (_cmap_segments[i].id_range_offset == 0)
    {
      return static_cast<std::uint16_t>(character_code +
                                        _cmap_segments[i].id_delta);
    }
    else
    {
      auto lookup_index = _cmap_segments[i].id_range_offset / 2 -
                          (_cmap_segments.size() - i) +
                          (character_code - _cmap_segments[i].start_code);
      return _cmap_glyph_indices[lookup_index];
    }
  }

  // Code should not be reached because the last segment should end with
  // code 0xffff, thus catching all possible input codes.
  BOOST_ASSERT(false);
  return 0;
}
}
