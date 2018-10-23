#include "shift/rc/action_font_import_ttf.h"
#include "shift/rc/ttf/font.h"
#include "shift/rc/resource_compiler_impl.h"
#include <shift/resource/mesh.h>
#include <shift/resource/buffer_view.h>
#include <shift/resource/font.h>
#include <shift/log/log.h>
#include <shift/math/line.h>
#include <shift/math/intersection.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <queue>
#include <memory>

namespace shift::rc
{
action_font_import_ttf::action_font_import_ttf()
: action_base(action_name, action_version)
{
}

bool action_font_import_ttf::process(resource_compiler_impl& compiler,
                                     job_description& job) const
{
  namespace fs = boost::filesystem;

  if (job.inputs.size() != 1)
  {
    log::error() << "The " << action_font_import_ttf::action_name
                 << " action can only process one input at a time.";
    return false;
  }
  const auto& input = job.inputs.at(0);

  if (!fs::exists(input->file->path) || !fs::is_regular_file(input->file->path))
  {
    log::error() << "Cannot find input file " << input->file->path << ".";
    return false;
  }

  ttf::font ttf;
  ttf.load(input->file->path);

  std::vector<glyph_t> glyph_meshes;
  for (const auto& glyph : ttf.glyphs())
  {
    const auto* contour_end_index = glyph.contour_ends.data();

    std::vector<point_t> packed_points;
    for (std::size_t i = 0; i != glyph.points.size(); ++i)
    {
      packed_points.push_back(
        {math::vector<2, float>{glyph.points[i].position.x * ttf.scale(),
                                glyph.points[i].position.y * ttf.scale()},
         glyph.points[i].on_curve ? point_flags_t{0}
                                  : point_flag_t::control_point});
    }

    glyph_t glyph_mesh;
    glyph_mesh.code = glyph.code;
    glyph_mesh.width = glyph.bounds.extent(0) * ttf.scale();
    glyph_mesh.height = glyph.bounds.extent(1) * ttf.scale();
    glyph_mesh.bearing_x = glyph.bearing_x * ttf.scale();
    glyph_mesh.bearing_y = glyph.bearing_y * ttf.scale();
    glyph_mesh.advance_x = glyph.advance_x * ttf.scale();
    glyph_mesh.advance_y = glyph.advance_y * ttf.scale();

    glyph_mesh.contours.resize(glyph.contours_count);
    short contour_start_index = 0;
    for (auto& contour : glyph_mesh.contours)
    {
      std::size_t start_index = glyph_mesh.points.size();
      std::size_t end_index;

      if (*contour_end_index - contour_start_index + 1 < 2)
      {
        // contour_start_index = *(contour_end_index++) + 1;
        continue;
      }

      std::size_t index;
      if (!(packed_points[contour_start_index].flags &
            point_flag_t::control_point))
      {
        glyph_mesh.points.push_back(packed_points[contour_start_index]);
        index = contour_start_index + 1;
      }
      else if (!(packed_points[contour_start_index + 1].flags &
                 point_flag_t::control_point))
      {
        // Point at contour_start_index is a control point. Process it
        // later.
        glyph_mesh.points.push_back(packed_points[contour_start_index + 1]);
        index = contour_start_index + 2;
      }
      else
      {
        // The first two contour points are control points. Thus create a
        // new intermediate point to start with.
        glyph_mesh.points.push_back(
          {(packed_points[contour_start_index].position +
            packed_points[contour_start_index + 1].position) *
             0.5f,
           point_flag_t::none});
        index = contour_start_index + 1;
      }

      bool previous_point_on_curve = true;
      for (; index <= *contour_end_index; ++index)
      {
        if (!(packed_points[index].flags & point_flag_t::control_point))
        {
          glyph_mesh.points.push_back(packed_points[index]);
          previous_point_on_curve = true;
        }
        else if (previous_point_on_curve)
        {
          glyph_mesh.points.push_back(packed_points[index]);
          previous_point_on_curve = false;
        }
        else
        {
          glyph_mesh.points.push_back({(packed_points[index - 1].position +
                                        packed_points[index].position) *
                                         0.5f,
                                       point_flags_t{0}});
          glyph_mesh.points.push_back(packed_points[index]);
        }
      }

      if (!(packed_points[contour_start_index].flags &
            point_flag_t::control_point))
      {
        // Either a line from index - 1 to contour_start_index or a curve
        // from index - 2 to contour_start_index with index - 1 as control
        // point.
        // glyph_mesh.points.push_back(packed_points[contour_start_index]);
      }
      else if (previous_point_on_curve)
      {
        glyph_mesh.points.push_back(packed_points[contour_start_index]);
        // glyph_mesh.points.push_back(packed_points[first_curve_point]);
      }
      else
      {
        // The last and first contour points are control points. Thus
        // append a new intermediate point to the stream.
        glyph_mesh.points.push_back(
          {(packed_points[index - 1].position +
            packed_points[contour_start_index].position) *
             0.5f,
           point_flags_t{0}});
        glyph_mesh.points.push_back(packed_points[contour_start_index]);
        // glyph_mesh.points.push_back(packed_points[first_curve_point]);
      }

      contour_start_index = *(contour_end_index++) + 1;
      end_index = glyph_mesh.points.size();

      for (auto p1i = start_index; p1i < end_index; ++p1i)
      {
        glyph_mesh.points[p1i].flags |= point_flag_t::inner;
        std::size_t p2i = p1i + 1;
        if (p2i >= end_index)
          p2i -= end_index - start_index;
        std::size_t p3i = p1i + 2;
        if (p3i >= end_index)
          p3i -= end_index - start_index;
        if (glyph_mesh.points[p2i].flags & point_flag_t::control_point)
        {
          auto& p1 = glyph_mesh.points[p1i];
          auto& p2 = glyph_mesh.points[p2i];
          auto& p3 = glyph_mesh.points[p3i];
          auto area =
            cross(p3.position - p1.position, p2.position - p1.position);
          // if (std::abs(area) < 10)
          //{
          //  contour.edges.push_back(glyph_mesh.edges.size());
          //  glyph_mesh.edges.emplace_back(p1i, p3i, edge_flag::border);
          //}
          // else
          {
            auto t = glyph_mesh.triangles.size();
            auto edge1_index = glyph_mesh.edges.size();
            glyph_mesh.edges.emplace_back(p2i, p3i, edge_flag::none, t);
            contour.edges.push_back(edge1_index);

            auto edge2_index = glyph_mesh.edges.size();
            glyph_mesh.edges.emplace_back(p3i, p1i, edge_flag::curve, t);
            contour.edges.push_back(edge2_index);

            auto edge3_index = glyph_mesh.edges.size();
            glyph_mesh.edges.emplace_back(p1i, p2i, edge_flag::none, t);
            contour.edges.push_back(edge3_index);

            auto& edge1 = glyph_mesh.edges[edge1_index];
            auto& edge2 = glyph_mesh.edges[edge2_index];
            auto& edge3 = glyph_mesh.edges[edge3_index];
            // Check triangle orientation.
            if (area > 0)
            {
              // The border flows along the long path.
              edge1.flags |= edge_flag::border;
              std::swap(edge1.neighbors[1], edge1.neighbors[0]);
              edge3.flags |= edge_flag::border;
              std::swap(edge3.neighbors[1], edge3.neighbors[0]);
              contour.triangles.emplace_back(t);
              glyph_mesh.triangles.emplace_back(
                p1i, p2i, p3i, triangle_flag::curve | triangle_flag::negate);
            }
            else
            {
              // The border flows along the short path.
              edge2.flags |= edge_flag::border;
              std::swap(edge2.neighbors[1], edge2.neighbors[0]);
              p2.flags |= point_flag_t::inner;
              // Fix triangle orientation.
              edge1.reverse();
              edge2.reverse();
              edge3.reverse();
              contour.triangles.emplace_back(t);
              glyph_mesh.triangles.emplace_back(p1i, p2i, p3i,
                                                triangle_flag::curve);
            }
          }
          p1i = p2i;
        }
        else
        {
          contour.edges.push_back(glyph_mesh.edges.size());
          glyph_mesh.edges.emplace_back(p2i, p1i, edge_flag::border);
        }
      }
    }

    // We need to make sure that there are no overlapping curve triangles,
    // as those result in graphical artifacts. We do this by adding
    // additional curve points, thus splitting said triangles into two
    // until no more overlaps occur.
    /// ToDo...

    glyph_meshes.push_back(glyph_mesh);
  }

  for (auto& glyph_mesh : glyph_meshes)
    triangulate(glyph_mesh);

  save(compiler, job, ttf, glyph_meshes);

  auto ttf_write_html = job.rule->options.find("ttf_write_html");
  if (ttf_write_html != job.rule->options.end() &&
      parser::json::get_if<bool>(&ttf_write_html->second) &&
      parser::json::get<bool>(ttf_write_html->second))
  {
    write_html(input->file->path, glyph_meshes);
  }

  return true;
}

void action_font_import_ttf::triangulate(glyph_t& glyph_mesh)
{
  using std::norm;

  for (std::size_t ebi = 0; ebi < glyph_mesh.edges.size(); ++ebi)
  {
    // Skip edges that already have two neighbors.
    if (glyph_mesh.edges[ebi].neighbors[1] !=
        std::numeric_limits<std::size_t>::max())
    {
      continue;
    }

    auto& edge = glyph_mesh.edges[ebi];
    std::size_t pai = edge.a;
    std::size_t pci = edge.b;
    const auto& pa = glyph_mesh.points[pai].position;
    const auto& pc = glyph_mesh.points[pci].position;

    // Build a sorted list of possible candidates for building a new
    // triangle.
    struct candidate
    {
      float weight;
      std::size_t pbi;
    };
    struct candidate_comperator
    {
      constexpr bool operator()(const candidate& lhs,
                                const candidate& rhs) const
      {
        return lhs.weight > rhs.weight;
      }
    };
    std::priority_queue<candidate, std::vector<candidate>, candidate_comperator>
      candidates;
    for (std::size_t pbi = 0; pbi < glyph_mesh.points.size(); ++pbi)
    {
      // Skip points pai and pci.
      if (pai == pbi || pci == pbi)
        continue;
      // Skip control points which are not on the inside.
      if (!(glyph_mesh.points[pbi].flags & point_flag_t::inner))
        continue;
      const auto& pb = glyph_mesh.points[pbi].position;

      auto area = cross(pa - pb, pc - pb);
      // Skip candidates in CCW order.
      if (area <= 0)
        continue;
      auto weight = norm(pa - pb) + norm(pb - pc) + norm(pc - pa);

      candidates.push({weight, pbi});
    }

    bool found_candidate = false;
    while (!candidates.empty() && !found_candidate)
    {
      /// ToDo: Do something with weight.
      // auto weight = candidates.top().weight;
      auto pbi = candidates.top().pbi;
      candidates.pop();
      const auto& pb = glyph_mesh.points[pbi].position;

      // Check if edges pc-pb and pb-pa already exist.
      std::size_t eai = std::numeric_limits<std::size_t>::max();
      std::size_t eci = std::numeric_limits<std::size_t>::max();
      for (std::size_t other_edge_index = 0;
           other_edge_index < glyph_mesh.edges.size(); ++other_edge_index)
      {
        const auto& other_edge = glyph_mesh.edges[other_edge_index];
        if ((other_edge.a == pci && other_edge.b == pbi) ||
            (other_edge.b == pci && other_edge.a == pbi))
        {
          eai = other_edge_index;
          continue;
        }
        if ((other_edge.a == pai && other_edge.b == pbi) ||
            (other_edge.b == pai && other_edge.a == pbi))
        {
          eci = other_edge_index;
          continue;
        }
      }
      // Skip point if one of the edges already has two neighbors.
      if ((eai != std::numeric_limits<std::size_t>::max() &&
           glyph_mesh.edges[eai].neighbors[1] !=
             std::numeric_limits<std::size_t>::max()) ||
          (eci != std::numeric_limits<std::size_t>::max() &&
           glyph_mesh.edges[eci].neighbors[1] !=
             std::numeric_limits<std::size_t>::max()))
      {
        continue;
      }

      bool collision = false;
      // Check if the potential new edge pb-pc collides with any other.
      for (const auto& other_edge : glyph_mesh.edges)
      {
        if (&edge == &other_edge)
          continue;
        // if (other_edge.flags & edge_flag::border)
        //  continue;
        // Skip test if either point is part of the other edge.
        if (other_edge.a == pci || other_edge.b == pci || other_edge.a == pbi ||
            other_edge.b == pbi)
        {
          continue;
        }
        if (math::intersection::rect_rect<true>(
              math::make_interval_from(pb, pc),
              math::make_interval_from(
                glyph_mesh.points[other_edge.a].position,
                glyph_mesh.points[other_edge.b].position)))
        {
          auto intersection = math::intersection::line_line(
            math::line<2, float>{pb, pc},
            math::line<2, float>{glyph_mesh.points[other_edge.a].position,
                                 glyph_mesh.points[other_edge.b].position});
          if (intersection == math::line_line_intersection::point ||
              intersection ==
                math::line_line_intersection::collinear_overlapping)
          {
            collision = true;
            break;
          }
        }
      }
      if (collision)
        continue;
      // Check if the potential new edge pa-pb collides with any other.
      for (const auto& other_edge : glyph_mesh.edges)
      {
        if (&edge == &other_edge)
          continue;
        // if (other_edge.flags & edge_flag::border)
        //  continue;
        // Skip test if either point is part of the other edge.
        if (other_edge.a == pai || other_edge.b == pai || other_edge.a == pbi ||
            other_edge.b == pbi)
        {
          continue;
        }
        if (math::intersection::rect_rect<true>(
              math::make_interval_from(pa, pb),
              math::make_interval_from(
                glyph_mesh.points[other_edge.a].position,
                glyph_mesh.points[other_edge.b].position)))
        {
          auto intersection = math::intersection::line_line(
            math::line<2, float>{pa, pb},
            math::line<2, float>{glyph_mesh.points[other_edge.a].position,
                                 glyph_mesh.points[other_edge.b].position});
          if (intersection == math::line_line_intersection::point ||
              intersection ==
                math::line_line_intersection::collinear_overlapping)
          {
            collision = true;
            break;
          }
        }
      }
      if (collision)
        continue;

      triangle_t new_triangle;
      new_triangle.a = pai;
      new_triangle.b = pbi;
      new_triangle.c = pci;

      // Insert new edges if they don't exist, yet.
      if (eai == std::numeric_limits<std::size_t>::max())
      {
        eai = glyph_mesh.edges.size();
        glyph_mesh.edges.emplace_back(new_triangle.b, new_triangle.c,
                                      edge_flag::none);
      }
      if (eci == std::numeric_limits<std::size_t>::max())
      {
        eci = glyph_mesh.edges.size();
        glyph_mesh.edges.emplace_back(new_triangle.a, new_triangle.b,
                                      edge_flag::none);
      }

      // Insert a new triangle.
      auto t = glyph_mesh.triangles.size();
      glyph_mesh.triangles.push_back(new_triangle);

      // Update all three edges with the new neighbor information.
      glyph_mesh.edges[eai].add_neighbor(t);
      glyph_mesh.edges[ebi].add_neighbor(t);
      glyph_mesh.edges[eci].add_neighbor(t);

      found_candidate = true;
    }
  }
}

void action_font_import_ttf::save(resource_compiler_impl& compiler,
                                  job_description& job, ttf::font& ttf,
                                  const std::vector<glyph_t>& glyph_meshes)
{
  auto font = std::make_shared<resource::font>();
  auto mesh = std::make_shared<resource::mesh>();
  font->mesh = mesh;
  font->ascent = ttf.horizontal_header().ascent * ttf.scale();
  font->descent = ttf.horizontal_header().descent * ttf.scale();
  font->line_gap = ttf.horizontal_header().line_gap * ttf.scale();

  struct glyph_vertex
  {
    math::vector2<float> position;
  };
  std::vector<glyph_vertex> vertices;
  auto vertex_buffer = std::make_shared<resource::buffer>();

  using index_t = std::uint32_t;
  std::vector<index_t> indices;
  auto index_buffer = std::make_shared<resource::buffer>();

  // Setup vertex layout.
  {
    resource::vertex_attribute position_attribute;
    position_attribute.vertex_buffer_view.buffer = vertex_buffer;
    position_attribute.vertex_buffer_view.offset = 0;
    position_attribute.offset = offsetof(glyph_vertex, position);
    position_attribute.size = sizeof(glyph_vertex::position);
    position_attribute.stride = sizeof(glyph_vertex);
    position_attribute.usage = resource::vertex_attribute_usage::position;
    position_attribute.component_type =
      resource::vertex_attribute_component_type::float32;
    position_attribute.data_type = resource::vertex_attribute_data_type::vec2;
    mesh->vertex_attributes.emplace_back(std::move(position_attribute));
  }
  // Setup index layout.
  mesh->index_buffer_view.buffer = index_buffer;
  mesh->index_buffer_view.offset = 0;
  mesh->index_data_type = resource::vertex_index_data_type::uint32;

  for (const auto& glyph_mesh : glyph_meshes)
  {
    resource::glyph glyph;

    auto first_vertex_index = vertices.size();
    for (const auto& point : glyph_mesh.points)
      vertices.push_back({point.position});

    auto first_solid_index = indices.size();
    std::uint32_t solid_index_count = 0;
    for (const auto& triangle : glyph_mesh.triangles)
    {
      if (!(triangle.flags & triangle_flag::curve))
      {
        indices.push_back(static_cast<index_t>(triangle.a));
        indices.push_back(static_cast<index_t>(triangle.b));
        indices.push_back(static_cast<index_t>(triangle.c));
        solid_index_count += 3;
      }
    }
    auto first_curve_index = indices.size();
    std::uint32_t curved_index_count = 0;
    for (const auto& triangle : glyph_mesh.triangles)
    {
      if (triangle.flags & triangle_flag::curve)
      {
        indices.push_back(static_cast<index_t>(triangle.a));
        indices.push_back(static_cast<index_t>(triangle.b));
        indices.push_back(static_cast<index_t>(triangle.c));
        curved_index_count += 3;
      }
    }

    if (solid_index_count)
    {
      resource::sub_mesh solid_sub_mesh{};
      solid_sub_mesh.topology = resource::primitive_topology::triangle_list;
      solid_sub_mesh.vertex_offset =
        static_cast<std::uint32_t>(first_vertex_index);
      solid_sub_mesh.first_index =
        static_cast<std::uint32_t>(first_solid_index);
      solid_sub_mesh.index_count = solid_index_count;
      mesh->sub_meshes.push_back(std::move(solid_sub_mesh));
      glyph.solid_sub_mesh_id =
        static_cast<std::uint32_t>(mesh->sub_meshes.size());
    }
    else
      glyph.solid_sub_mesh_id = 0;

    if (curved_index_count)
    {
      resource::sub_mesh curved_sub_mesh{};
      curved_sub_mesh.topology = resource::primitive_topology::triangle_list;
      curved_sub_mesh.vertex_offset =
        static_cast<std::uint32_t>(first_vertex_index);
      curved_sub_mesh.first_index =
        static_cast<std::uint32_t>(first_curve_index);
      curved_sub_mesh.index_count = curved_index_count;
      mesh->sub_meshes.push_back(std::move(curved_sub_mesh));
      glyph.curved_sub_mesh_id =
        static_cast<std::uint32_t>(mesh->sub_meshes.size());
    }
    else
      glyph.curved_sub_mesh_id = 0;

    glyph.width = glyph_mesh.width;
    glyph.height = glyph_mesh.height;
    glyph.bearing_x = glyph_mesh.bearing_x;
    glyph.bearing_y = glyph_mesh.bearing_y;
    glyph.advance_x = glyph_mesh.advance_x;
    glyph.advance_y = glyph_mesh.advance_y;
    if (!font->glyphs.insert({glyph_mesh.code, std::move(glyph)}).second)
    {
      /// ToDo: Ignore this case for now, but replace glyph map with a glyph
      /// vector plus a translation table similat to the TTF cmap;
    }
  }

  vertex_buffer->storage.resize(vertices.size() * sizeof(glyph_vertex));
  std::memcpy(vertex_buffer->storage.data(), vertices.data(),
              vertex_buffer->storage.size());
  mesh->vertex_attributes.front().vertex_buffer_view.size =
    static_cast<std::uint32_t>(vertex_buffer->storage.size());
  for (auto& attribute : mesh->vertex_attributes)
    attribute.vertex_buffer_view.buffer.update_id();

  index_buffer->storage.resize(indices.size() * sizeof(index_t));
  std::memcpy(index_buffer->storage.data(), indices.data(),
              index_buffer->storage.size());
  mesh->index_buffer_view.size =
    static_cast<std::uint32_t>(index_buffer->storage.size());
  mesh->index_buffer_view.buffer.update_id();

  compiler.save(*vertex_buffer, job.output("vertex_buffer", {}), job);
  compiler.save(*index_buffer, job.output("index_buffer", {}), job);
  font->mesh.update_id();
  compiler.save(*font->mesh, job.output("mesh", {}), job);
  compiler.save(*font, job.output("font", {}), job);
}

void action_font_import_ttf::write_html(
  const boost::filesystem::path& input_filename,
  const std::vector<glyph_t>& glyph_meshes)
{
  std::ofstream test_file;
  std::stringstream test_filename;
  test_filename
    << "public/fonts/"
    << input_filename.filename().replace_extension(".html").string();
  test_file.open(test_filename.str(), std::ios::out | std::ios::trunc);
  test_file << R"(<!DOCTYPE html><html><body style="background-color:#000">)"
            << std::endl;
  for (const auto& glyph_mesh : glyph_meshes)
  {
    test_file << R"(<canvas id="glyph)" << glyph_mesh.code << R"(" width=")"
              << std::ceil(glyph_mesh.width + 20) << R"(" height=")"
              << std::ceil(glyph_mesh.height + 20)
              << R"(" style="border:1px solid #111;"></canvas>)" << std::endl;
  }
  test_file << "<script>" << std::endl;
  std::size_t num_glyphs = 0;
  for (const auto& glyph_mesh : glyph_meshes)
  {
    if (++num_glyphs > 128)
      break;
    test_file << "{" << std::endl;
    test_file << R"(var c = document.getElementById("glyph)" << glyph_mesh.code
              << R"(");)" << std::endl;
    test_file << R"(var ctx = c.getContext("2d");)" << std::endl;
    test_file << R"(ctx.fillStyle = "#000";)" << std::endl;
    test_file << "ctx.fillRect(0, 0, " << std::ceil(glyph_mesh.width) << ", "
              << std::ceil(glyph_mesh.height) << ");" << std::endl;
    test_file << R"-(ctx.fillStyle = "rgba(0, 0, 128, 0.5)";)-" << std::endl;
    for (const auto& triangle : glyph_mesh.triangles)
    {
      test_file << "ctx.beginPath();" << std::endl;
      test_file << "  ctx.moveTo("
                << (glyph_mesh.points[triangle.a].position.x -
                    glyph_mesh.bearing_x + 10)
                << ", "
                << (glyph_mesh.bearing_y -
                    glyph_mesh.points[triangle.a].position.y + 10)
                << ");" << std::endl;
      if (triangle.flags & triangle_flag::curve)
      {
        test_file << "  ctx.quadraticCurveTo("
                  << (glyph_mesh.points[triangle.b].position.x -
                      glyph_mesh.bearing_x + 10)
                  << ", "
                  << (glyph_mesh.bearing_y -
                      glyph_mesh.points[triangle.b].position.y + 10)
                  << ", "
                  << (glyph_mesh.points[triangle.c].position.x -
                      glyph_mesh.bearing_x + 10)
                  << ", "
                  << (glyph_mesh.bearing_y -
                      glyph_mesh.points[triangle.c].position.y + 10)
                  << ");" << std::endl;
      }
      else
      {
        test_file << "  ctx.lineTo("
                  << (glyph_mesh.points[triangle.c].position.x -
                      glyph_mesh.bearing_x + 10)
                  << ", "
                  << (glyph_mesh.bearing_y -
                      glyph_mesh.points[triangle.c].position.y + 10)
                  << ");" << std::endl;
      }
      if (!(triangle.flags & triangle_flag::curve &&
            triangle.flags & triangle_flag::negate))
      {
        test_file << "  ctx.lineTo("
                  << (glyph_mesh.points[triangle.b].position.x -
                      glyph_mesh.bearing_x + 10)
                  << ", "
                  << (glyph_mesh.bearing_y -
                      glyph_mesh.points[triangle.b].position.y + 10)
                  << ");" << std::endl;
      }
      test_file << "  ctx.closePath();" << std::endl;
      test_file << "ctx.fill();" << std::endl;
    }
    for (const auto& edge : glyph_mesh.edges)
    {
      if (edge.flags & edge_flag::border)
        test_file << "ctx.strokeStyle = '#F00';" << std::endl;
      else if (edge.flags & edge_flag::curve)
        test_file << "ctx.strokeStyle = '#0FF';" << std::endl;
      else
        test_file << "ctx.strokeStyle = '#F0F';" << std::endl;
      test_file << "ctx.beginPath();" << std::endl;
      test_file << "  ctx.moveTo("
                << (glyph_mesh.points[edge.a].position.x -
                    glyph_mesh.bearing_x + 10)
                << ", "
                << (glyph_mesh.bearing_y -
                    glyph_mesh.points[edge.a].position.y + 10)
                << ");" << std::endl;
      test_file << "  ctx.lineTo("
                << (glyph_mesh.points[edge.b].position.x -
                    glyph_mesh.bearing_x + 10)
                << ", "
                << (glyph_mesh.bearing_y -
                    glyph_mesh.points[edge.b].position.y + 10)
                << ");" << std::endl;
      test_file << "ctx.stroke();" << std::endl;
    }
    test_file << "}" << std::endl;
  }
  test_file << "</script></body></html>" << std::endl;
  test_file.close();
}
}
