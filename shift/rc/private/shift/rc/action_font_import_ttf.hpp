#ifndef SHIFT_RC_ACTION_FONT_IMPORT_TTF_HPP
#define SHIFT_RC_ACTION_FONT_IMPORT_TTF_HPP

#include <vector>
#include <map>
#include <shift/core/exception.hpp>
#include <shift/core/bit_field.hpp>
#include <shift/math/vector.hpp>
#include <shift/parser/json/json.hpp>
//#include <shift/serialization2/all.hpp>
#include <shift/resource_db/repository.hpp>
#include "shift/rc/types.hpp"

namespace shift::rc::ttf
{
class font;
};

namespace shift::rc
{
struct edge_t;
struct triangle_t;

enum class point_flag_t
{
  none = 0b0000,
  /// Point is used as a curve control point.
  control_point = 0b0001,
  /// Point is on the inside of the glyph.
  inner = 0b0010
};
using point_flags_t = core::bit_field<point_flag_t>;

struct point_t
{
  math::vector2<float> position;
  point_flags_t flags;
};

enum class edge_flag
{
  none = 0x00,
  /// An edge on the outer border of a glyph (thus having only one
  /// neigbor).
  border = 0x01,
  /// An invisible edge substituted with a curve.
  curve = 0x02
};
using edge_flags = core::bit_field<edge_flag>;

struct edge_t
{
  /// Constructor.
  edge_t(std::size_t a, std::size_t b, edge_flags flags,
         std::size_t n1 = std::numeric_limits<std::size_t>::max(),
         std::size_t n2 = std::numeric_limits<std::size_t>::max())
  : a(a), b(b), flags(flags)
  {
    /// ToDo: error C2797: list initialization inside member initializer
    /// list or non-static data member initializer is not implemented
    neighbors[0] = n1;
    neighbors[1] = n2;
  }

  ///
  inline void add_neighbor(std::size_t triangle_index)
  {
    if (neighbors[0] == std::numeric_limits<std::size_t>::max() &&
        !(flags & edge_flag::border))
    {
      neighbors[0] = triangle_index;
    }
    else if (neighbors[1] == std::numeric_limits<std::size_t>::max())
      neighbors[1] = triangle_index;
    else
    {
      // You tried to add more than two neighbors.
      BOOST_ASSERT((neighbors[0] == std::numeric_limits<std::size_t>::max() &&
                    !(flags & edge_flag::border)) ||
                   neighbors[1] == std::numeric_limits<std::size_t>::max());
    }
  }

  ///
  inline void removeNeighbor(std::size_t triangle_index)
  {
    if (neighbors[1] == triangle_index)
      neighbors[1] = std::numeric_limits<std::size_t>::max();
    else if (neighbors[0] == triangle_index)
      neighbors[0] = std::numeric_limits<std::size_t>::max();
    else
    {
      // The specified triangle is no neighbor.
      BOOST_ASSERT(neighbors[1] == triangle_index ||
                   neighbors[0] == triangle_index);
    }
  }

  inline void reverse()
  {
    std::swap(a, b);
  }

  std::size_t a;
  std::size_t b;
  edge_flags flags;
  /// Triangle indices, with std::numeric_limits<std::size_t>::max()
  /// determining no neighbor.
  std::array<std::size_t, 2> neighbors;
};

enum class triangle_flag
{
  none = 0x00,
  curve = 0x01,
  negate = 0x02
};
using triangle_flags = core::bit_field<triangle_flag>;

inline triangle_flags operator|(triangle_flag lhs, triangle_flag rhs)
{
  return triangle_flags{lhs} | rhs;
}

struct triangle_t
{
  /// Default constructor.
  triangle_t() = default;

  /// Constructor.
  triangle_t(std::size_t a, std::size_t b, std::size_t c, triangle_flags flags)
  : a(a), b(b), c(c), flags(flags)
  {
  }

  /// For curve triangles, this is the start point index.
  std::size_t a = std::numeric_limits<std::size_t>::max();
  /// For curve triangles, this is the control point index.
  std::size_t b = std::numeric_limits<std::size_t>::max();
  /// For curve triangles, this is the end point index.
  std::size_t c = std::numeric_limits<std::size_t>::max();
  triangle_flags flags = triangle_flags{0};
};

struct contour_t
{
  /// Edge indices.
  std::vector<std::size_t> edges;
  /// Triangle indices.
  std::vector<std::size_t> triangles;
};

struct glyph_t
{
  unsigned long code;
  float width;
  float height;
  float bearing_x;
  float bearing_y;
  float advance_x;
  float advance_y;
  std::vector<point_t> points;
  std::vector<edge_t> edges;
  std::vector<triangle_t> triangles;
  std::vector<contour_t> contours;
};

class action_font_import_ttf : public action_base,
                               public core::singleton<action_font_import_ttf>
{
public:
  static constexpr const char* action_name = "font-import-ttf";
  static constexpr const char* action_version = "1.0.0001";

public:
  /// Default constructor.
  action_font_import_ttf();

  /// @see action_base::process.
  bool process(resource_compiler_impl& compiler,
               job_description& job) const override;

private:
  ///
  static void triangulate(glyph_t& glyph_mesh);

  /// Stores all information to file.
  static void save(resource_compiler_impl& compiler, job_description& job,
                   ttf::font& ttf, const std::vector<glyph_t>& glyph_meshes);

  /// Write HTML5 document for debugging purposes.
  static void write_html(const std::filesystem::path& input_filename,
                         const std::vector<glyph_t>& glyph_meshes);
};
}

#endif
