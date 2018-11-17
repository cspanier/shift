#include "shift/rc/action_mesh_import_ply.hpp"
#include "shift/rc/optimizer_mesh/filter.hpp"
#include "shift/rc/resource_compiler_impl.hpp"
#include <shift/resource/mesh.hpp>
#include <shift/log/log.hpp>
#include <shift/math/vector.hpp>
#include <boost/filesystem.hpp>
#include <boost/endian/conversion.hpp>
#include <queue>
#include <memory>

namespace shift::rc
{
namespace fs = boost::filesystem;

enum class vertex_property_type
{
  undefined,
  uint8,
  uint16,
  uint32,
  float32
};

enum class vertex_property_name
{
  x,
  y,
  z,
  nx,
  ny,
  nz,
  u,
  v
};

struct vertex_property
{
  vertex_property_type type;
  vertex_property_name name;
  std::size_t offset;
};

action_mesh_import_ply::action_mesh_import_ply()
: action_base(action_name, action_version)
{
}

bool action_mesh_import_ply::process(resource_compiler_impl& compiler,
                                     job_description& job) const
{
  using std::get;
  using std::get_if;

  if (job.inputs.size() != 1)
  {
    log::error() << "The " << action_mesh_import_ply::action_name
                 << " action can only process one input at a time.";
    return false;
  }
  auto& input = *job.inputs.begin()->second;

  if (!fs::exists(input.file->path) || !fs::is_regular_file(input.file->path))
  {
    log::error() << "Cannot find input file " << input.file->path << ".";
    return false;
  }

  std::ifstream file;
  file.open(input.file->generic_string,
            std::ios_base::in | std::ios_base::binary);
  if (!file.is_open())
  {
    /// ToDo: Throw exception.
    return false;
  }

  float scale = 1.0f;
  auto scale_iter = job.rule->options.find("scale");
  if (scale_iter != job.rule->options.end() &&
      get_if<double>(&scale_iter->second))
  {
    scale = static_cast<float>(get<double>(scale_iter->second));
  }

  auto get_option = [&](const std::string& name, auto default_value) {
    auto options_iter = job.rule->options.find(name);
    if (options_iter != job.rule->options.end() &&
        get_if<decltype(default_value)>(&options_iter->second))
    {
      return get<decltype(default_value)>(options_iter->second);
    }
    else
      return default_value;
  };

  auto swap_x_y = get_option("swap-x-y", false);
  auto swap_y_z = get_option("swap-y-z", false);
  auto swap_x_z = get_option("swap-x-z", false);
  auto flip_x = get_option("flip-x", false);
  auto flip_y = get_option("flip-y", false);
  auto flip_z = get_option("flip-z", false);
  auto flip_nx = get_option("flip-nx", false);
  auto flip_ny = get_option("flip-ny", false);
  auto flip_nz = get_option("flip-nz", false);
  auto flip_u = get_option("flip-u", false);
  auto flip_v = get_option("flip-v", false);
  auto flip_indices = get_option("flip-indices", false);

  auto property_size = [](const vertex_property_type& type) {
    switch (type)
    {
    case vertex_property_type::uint8:
      return 1;

    case vertex_property_type::uint16:
      return 2;

    case vertex_property_type::uint32:
    case vertex_property_type::float32:
      return 4;

    default:
      return 0;
    }
  };

  bool valid_header = false;
  auto endian = boost::endian::order::native;
  bool has_vertex_element = false;
  bool has_indices = false;
  std::size_t num_vertices = 0;
  std::vector<vertex_property> vertex_layout;
  std::size_t vertex_property_offset = 0;
  std::size_t num_faces = 0;
  vertex_property_type index_list_type = vertex_property_type::undefined;
  vertex_property_type index_element_type = vertex_property_type::undefined;
  for (std::array<char, 256> line_buffer{};
       file.getline(line_buffer.data(), line_buffer.size(), '\n');)
  {
    std::stringstream line(line_buffer.data());
    std::string token;
    line >> token;
    if (token == "ply")
    {
      valid_header = true;
      continue;
    }
    else if (!valid_header)
    {
      log::error() << "No valid PLY header found: ";
      return false;
    }
    else if (token == "format")
    {
      if (line.str() == "format binary_little_endian 1.0")
        endian = boost::endian::order::little;
      else if (line.str() == "format binary_big_endian 1.0")
        endian = boost::endian::order::big;
      else
      {
        log::error() << "Unsupported PLY format: " << line.str();
        return false;
      }
    }
    else if (token == "element")
    {
      line >> token;
      if (token == "vertex")
      {
        if (has_vertex_element)
        {
          log::error() << "Cannot specify multiple vertex definitions";
          return false;
        }
        has_vertex_element = true;
        line >> num_vertices;
      }
      else if (token == "face")
      {
        if (has_indices)
        {
          log::error() << "Cannot specify multiple vertex index lists";
          return false;
        }
        has_indices = true;
        line >> num_faces;
      }
      else
      {
        log::error() << "Unknown PLY element type: " << line.str();
        return false;
      }
    }
    else if (token == "property")
    {
      line >> token;
      if (token == "float")
      {
        line >> token;
        if (token == "x")
        {
          vertex_layout.push_back({vertex_property_type::float32,
                                   vertex_property_name::x,
                                   vertex_property_offset});
        }
        else if (token == "y")
        {
          vertex_layout.push_back({vertex_property_type::float32,
                                   vertex_property_name::y,
                                   vertex_property_offset});
        }
        else if (token == "z")
        {
          vertex_layout.push_back({vertex_property_type::float32,
                                   vertex_property_name::z,
                                   vertex_property_offset});
        }
        else if (token == "nx")
        {
          vertex_layout.push_back({vertex_property_type::float32,
                                   vertex_property_name::nx,
                                   vertex_property_offset});
        }
        else if (token == "ny")
        {
          vertex_layout.push_back({vertex_property_type::float32,
                                   vertex_property_name::ny,
                                   vertex_property_offset});
        }
        else if (token == "nz")
        {
          vertex_layout.push_back({vertex_property_type::float32,
                                   vertex_property_name::nz,
                                   vertex_property_offset});
        }
        else if (token == "u" || token == "s")
        {
          vertex_layout.push_back({vertex_property_type::float32,
                                   vertex_property_name::u,
                                   vertex_property_offset});
        }
        else if (token == "v" || token == "t")
        {
          vertex_layout.push_back({vertex_property_type::float32,
                                   vertex_property_name::v,
                                   vertex_property_offset});
        }
        else
        {
          log::error() << "Unsupported vertex property: " << line.str();
          return false;
        }
        vertex_property_offset += property_size(vertex_property_type::float32);
      }
      else if (token == "list")
      {
        line >> token;
        vertex_property_type list_type = vertex_property_type::undefined;
        if (token == "uchar" || token == "uint8")
          list_type = vertex_property_type::uint8;
        else if (token == "ushort" || token == "uint16")
          list_type = vertex_property_type::uint16;
        else if (token == "uint" || token == "uint32")
          list_type = vertex_property_type::uint32;
        else
        {
          log::error() << "Unsupported vertex index list type: " << line.str();
          return false;
        }

        line >> token;
        vertex_property_type element_type = vertex_property_type::undefined;
        if (token == "uchar" || token == "char" || token == "uint8")
          element_type = vertex_property_type::uint8;
        else if (token == "ushort" || token == "short" || token == "uint16")
          element_type = vertex_property_type::uint16;
        else if (token == "uint" || token == "int" || token == "uint32")
          element_type = vertex_property_type::uint32;
        else
        {
          log::error() << "Unsupported vertex element type: " << line.str();
          return false;
        }

        line >> token;
        if (token == "vertex_indices")
        {
          index_list_type = list_type;
          index_element_type = element_type;
        }
        else
        {
          log::error() << "Expected 'vertex_indices' at end of line: "
                       << line.str();
          return false;
        }
      }
      else
      {
        log::error() << "Unsupported property type: " << line.str();
        return false;
      }
    }
    else if (token == "comment")
      continue;
    else if (token == "end_header")
      break;
    else
      log::warning() << "Unknown PLY header: " << line.str();
  }
  if (file.fail())
  {
    log::error() << "Unexpected file read error.";
    return false;
  }

  if (!has_vertex_element)
  {
    log::error() << "PLY files without vertex definitions are not supported.";
    return false;
  }
  if (!has_indices)
  {
    log::error() << "PLY files without vertex indices are not supported.";
    return false;
  }

  auto property_index = [&](vertex_property_name name) {
    int index = 0;
    for (const auto& property : vertex_layout)
    {
      if (property.name == name)
        return index;
      else
        ++index;
    }
    return -1;
  };
  auto index_x = property_index(vertex_property_name::x);
  auto index_y = property_index(vertex_property_name::y);
  auto index_z = property_index(vertex_property_name::z);
  auto index_nx = property_index(vertex_property_name::nx);
  auto index_ny = property_index(vertex_property_name::ny);
  auto index_nz = property_index(vertex_property_name::nz);
  auto index_u = property_index(vertex_property_name::u);
  auto index_v = property_index(vertex_property_name::v);
  if (swap_x_y)
  {
    std::swap(index_x, index_y);
    std::swap(index_nx, index_ny);
  }
  if (swap_y_z)
  {
    std::swap(index_y, index_z);
    std::swap(index_ny, index_nz);
  }
  if (swap_x_z)
  {
    std::swap(index_x, index_z);
    std::swap(index_nx, index_nz);
  }
  if (index_x < 0 || index_y < 0 || index_z < 0)
  {
    log::error() << "We only support 3D position vectors.";
    return false;
  }
  if (index_nx >= 0 && (index_ny < 0 || index_nz < 0))
  {
    log::error() << "We only support 3D normal vectors.";
    return false;
  }
  if (index_u >= 0 && index_v < 0)
  {
    log::error() << "We only support 2D texture coordinates.";
    return false;
  }
  if ((index_x >= 0 && index_y >= 0 && index_z >= 0 &&
       (vertex_layout[index_x].type != vertex_layout[index_y].type ||
        vertex_layout[index_x].type != vertex_layout[index_z].type)) ||
      (index_nx >= 0 && index_ny >= 0 && index_nz >= 0 &&
       (vertex_layout[index_nx].type != vertex_layout[index_ny].type ||
        vertex_layout[index_nx].type != vertex_layout[index_nz].type)) ||
      (index_u >= 0 && index_v >= 0 &&
       vertex_layout[index_u].type != vertex_layout[index_v].type))
  {
    log::error() << "Vector components must all be the same type.";
    return false;
  }
  std::uint32_t vertex_size = 0;
  for (const auto& property : vertex_layout)
    vertex_size += property_size(property.type);
  std::uint32_t index_element_size = property_size(index_element_type);

  auto mesh = std::make_shared<resource::mesh>();

  auto index_buffer = std::make_shared<resource::buffer>();

  mesh->index_buffer_view.buffer = index_buffer;
  mesh->index_buffer_view.offset = 0;
  mesh->index_buffer_view.size = 0;
  switch (index_element_type)
  {
  case vertex_property_type::uint8:
    mesh->index_data_type = resource::vertex_index_data_type::uint8;
    break;

  case vertex_property_type::uint16:
    mesh->index_data_type = resource::vertex_index_data_type::uint16;
    break;

  case vertex_property_type::uint32:
    mesh->index_data_type = resource::vertex_index_data_type::uint32;
    break;

  default:
    BOOST_ASSERT(false);
    return false;
  }

  auto vertex_buffer = std::make_shared<resource::buffer>();
  resource::buffer_view position_buffer_view;
  position_buffer_view.buffer = vertex_buffer;
  position_buffer_view.offset = 0u;
  position_buffer_view.size = 0u;

  resource::buffer_view normal_buffer_view;
  normal_buffer_view.buffer = vertex_buffer;
  normal_buffer_view.offset = 0u;
  normal_buffer_view.size = 0u;

  resource::buffer_view texcoord_buffer_view;
  texcoord_buffer_view.buffer = vertex_buffer;
  texcoord_buffer_view.offset = 0u;
  texcoord_buffer_view.size = 0u;

  if (index_x >= 0 && index_y >= 0 && index_z >= 0)
  {
    resource::vertex_attribute position_attribute;
    position_attribute.offset = 0u;
    position_attribute.stride = 3 * sizeof(float);
    position_attribute.usage = resource::vertex_attribute_usage::position;
    position_attribute.component_type =
      resource::vertex_attribute_component_type::float32;
    position_attribute.data_type = resource::vertex_attribute_data_type::vec3;
    position_attribute.size = resource::vertex_attribute_size(
      position_attribute.component_type, position_attribute.data_type);

    position_buffer_view.offset = vertex_buffer->storage.size();
    position_buffer_view.size = num_vertices * position_attribute.stride;
    vertex_buffer->storage.resize(position_buffer_view.offset +
                                  position_buffer_view.size);
    position_attribute.vertex_buffer_view = position_buffer_view;

    mesh->vertex_attributes.push_back(std::move(position_attribute));
  }
  if (index_nx >= 0 && index_ny >= 0 && index_nz >= 0)
  {
    resource::vertex_attribute normal_attribute;
    normal_attribute.offset = 0u;
    normal_attribute.stride = 3 * sizeof(float);
    normal_attribute.usage = resource::vertex_attribute_usage::normal;
    normal_attribute.component_type =
      resource::vertex_attribute_component_type::float32;
    normal_attribute.data_type = resource::vertex_attribute_data_type::vec3;
    normal_attribute.size = resource::vertex_attribute_size(
      normal_attribute.component_type, normal_attribute.data_type);

    normal_buffer_view.offset = vertex_buffer->storage.size();
    normal_buffer_view.size = num_vertices * normal_attribute.stride;
    vertex_buffer->storage.resize(normal_buffer_view.offset +
                                  normal_buffer_view.size);
    normal_attribute.vertex_buffer_view = normal_buffer_view;

    mesh->vertex_attributes.push_back(std::move(normal_attribute));
  }
  if (index_u >= 0 && index_v >= 0)
  {
    resource::vertex_attribute texcoord_attribute;
    texcoord_attribute.offset = 0u;
    texcoord_attribute.stride = 2 * sizeof(float);
    texcoord_attribute.usage = resource::vertex_attribute_usage::texcoord;
    texcoord_attribute.component_type =
      resource::vertex_attribute_component_type::float32;
    texcoord_attribute.data_type = resource::vertex_attribute_data_type::vec2;
    texcoord_attribute.size = resource::vertex_attribute_size(
      texcoord_attribute.component_type, texcoord_attribute.data_type);

    texcoord_buffer_view.offset = vertex_buffer->storage.size();
    texcoord_buffer_view.size = num_vertices * texcoord_attribute.stride;
    vertex_buffer->storage.resize(texcoord_buffer_view.offset +
                                  texcoord_buffer_view.size);
    texcoord_attribute.vertex_buffer_view = texcoord_buffer_view;

    mesh->vertex_attributes.push_back(std::move(texcoord_attribute));
  }

  std::vector<char> vertex_cache;
  vertex_cache.resize(vertex_size);
  math::vector3<float> min =
    math::make_vector_from(std::numeric_limits<float>::infinity(),
                           std::numeric_limits<float>::infinity(),
                           std::numeric_limits<float>::infinity());
  math::vector3<float> max =
    math::make_vector_from(-std::numeric_limits<float>::infinity(),
                           -std::numeric_limits<float>::infinity(),
                           -std::numeric_limits<float>::infinity());
  for (std::size_t i = 0; i < num_vertices; ++i)
  {
    file.read(vertex_cache.data(), vertex_cache.size());
    if (!file)
    {
      log::error() << "Unexpected end of file.";
      return false;
    }
    for (const auto& attribute : mesh->vertex_attributes)
    {
      if (attribute.usage == resource::vertex_attribute_usage::position &&
          attribute.component_type ==
            resource::vertex_attribute_component_type::float32 &&
          attribute.data_type == resource::vertex_attribute_data_type::vec3)
      {
        auto& target = *reinterpret_cast<math::vector3<float>*>(
          vertex_buffer->storage.data() + position_buffer_view.offset +
          i * attribute.stride + attribute.offset);
        std::uint32_t value;
        value = boost::endian::conditional_reverse(
          *reinterpret_cast<std::uint32_t*>(vertex_cache.data() +
                                            vertex_layout[index_x].offset),
          endian, boost::endian::order::native);
        target.x = *reinterpret_cast<const float*>(&value);
        if (flip_x)
          target.x = -target.x;

        value = boost::endian::conditional_reverse(
          *reinterpret_cast<std::uint32_t*>(vertex_cache.data() +
                                            vertex_layout[index_y].offset),
          endian, boost::endian::order::native);
        target.y = *reinterpret_cast<const float*>(&value);
        if (flip_y)
          target.y = -target.y;

        value = boost::endian::conditional_reverse(
          *reinterpret_cast<std::uint32_t*>(vertex_cache.data() +
                                            vertex_layout[index_z].offset),
          endian, boost::endian::order::native);
        target.z = *reinterpret_cast<const float*>(&value);
        if (flip_z)
          target.z = -target.z;

        if (attribute.usage == resource::vertex_attribute_usage::position)
          target *= scale;

        min = math::min(min, target);
        max = math::max(max, target);
      }
      else if (attribute.usage == resource::vertex_attribute_usage::normal &&
               attribute.component_type ==
                 resource::vertex_attribute_component_type::float32 &&
               attribute.data_type ==
                 resource::vertex_attribute_data_type::vec3)
      {
        auto& target = *reinterpret_cast<math::vector3<float>*>(
          vertex_buffer->storage.data() + normal_buffer_view.offset +
          i * attribute.stride + attribute.offset);
        std::uint32_t value;
        value = boost::endian::conditional_reverse(
          *reinterpret_cast<std::uint32_t*>(vertex_cache.data() +
                                            vertex_layout[index_nx].offset),
          endian, boost::endian::order::native);
        target.x = *reinterpret_cast<const float*>(&value);
        if (flip_nx)
          target.x = -target.x;

        value = boost::endian::conditional_reverse(
          *reinterpret_cast<std::uint32_t*>(vertex_cache.data() +
                                            vertex_layout[index_ny].offset),
          endian, boost::endian::order::native);
        target.y = *reinterpret_cast<const float*>(&value);
        if (flip_ny)
          target.y = -target.y;

        value = boost::endian::conditional_reverse(
          *reinterpret_cast<std::uint32_t*>(vertex_cache.data() +
                                            vertex_layout[index_nz].offset),
          endian, boost::endian::order::native);
        target.z = *reinterpret_cast<const float*>(&value);
        if (flip_nz)
          target.z = -target.z;
      }
      else if (attribute.usage == resource::vertex_attribute_usage::texcoord &&
               attribute.component_type ==
                 resource::vertex_attribute_component_type::float32 &&
               attribute.data_type ==
                 resource::vertex_attribute_data_type::vec2)
      {
        auto& target = *reinterpret_cast<math::vector2<float>*>(
          vertex_buffer->storage.data() + texcoord_buffer_view.offset +
          i * attribute.stride + attribute.offset);
        std::uint32_t value;
        value = boost::endian::conditional_reverse(
          *reinterpret_cast<std::uint32_t*>(vertex_cache.data() +
                                            vertex_layout[index_u].offset),
          endian, boost::endian::order::native);
        target.x = *reinterpret_cast<const float*>(&value);
        if (flip_u)
          target.x = 1.0f - target.x;

        value = boost::endian::conditional_reverse(
          *reinterpret_cast<std::uint32_t*>(vertex_cache.data() +
                                            vertex_layout[index_v].offset),
          endian, boost::endian::order::native);
        target.y = *reinterpret_cast<const float*>(&value);
        if (flip_v)
          target.y = 1.0f - target.y;
      }
      else
      {
        log::error() << "Unsupported component type.";
        return false;
      }
    }
    // vertex_cache.data()
  }
  mesh->bounding_box = math::make_aabb_from_min_max<3, float>(min, max);

  resource::sub_mesh sub_mesh;
  sub_mesh.topology = resource::primitive_topology::triangle_list;
  sub_mesh.vertex_offset = 0;
  sub_mesh.first_index = 0;
  sub_mesh.index_count = 0;

  auto current_pos = file.tellg();
  file.seekg(0, std::ios_base::end);
  auto end_pos = file.tellg();
  file.seekg(current_pos, std::ios_base::beg);
  index_buffer->storage.resize(
    3 * (static_cast<std::size_t>(end_pos - current_pos) -
         num_faces * property_size(index_list_type) -
         2 * num_faces * property_size(index_element_type)));

  auto load_index = [&](auto& value) {
    file.read(reinterpret_cast<char*>(&value), sizeof(value));
    boost::endian::conditional_reverse_inplace(value, endian,
                                               boost::endian::order::native);
  };

  auto store_index = [&](auto value) {
    *reinterpret_cast<decltype(value)*>(index_buffer->storage.data() +
                                        mesh->index_buffer_view.size) = value;
    mesh->index_buffer_view.size += sizeof(value);
  };

  flip_indices ^= swap_x_y ^ swap_y_z ^ swap_x_z ^ flip_x ^ flip_y ^ flip_z;
  for (std::size_t i = 0; i < num_faces; ++i)
  {
    std::uint32_t num_face_indices = 0;
    switch (index_list_type)
    {
    case vertex_property_type::uint8:
    {
      std::uint8_t indices;
      file.read(reinterpret_cast<char*>(&indices), sizeof(indices));
      num_face_indices = indices;
      break;
    }

    case vertex_property_type::uint16:
    {
      std::uint16_t indices;
      file.read(reinterpret_cast<char*>(&indices), sizeof(indices));
      boost::endian::conditional_reverse_inplace(indices, endian,
                                                 boost::endian::order::native);
      num_face_indices = indices;
      break;
    }

    case vertex_property_type::uint32:
    {
      std::uint32_t indices;
      file.read(reinterpret_cast<char*>(&indices), sizeof(indices));
      boost::endian::conditional_reverse_inplace(indices, endian,
                                                 boost::endian::order::native);
      num_face_indices = indices;
      break;
    }

    default:
      BOOST_ASSERT(false);
      return false;
    }

    if (num_face_indices < 3)
    {
      log::error() << "Faces need at least 3 vertices.";
      return false;
    }

    switch (index_element_type)
    {
    case vertex_property_type::uint8:
    {
      std::uint8_t index_a;
      std::uint8_t index_b;
      std::uint8_t index_c;
      load_index(index_a);
      load_index(index_b);
      if (flip_indices)
        std::swap(index_a, index_b);
      for (std::size_t index = 2; index < num_face_indices; ++index)
      {
        load_index(index_c);
        store_index(index_a);
        store_index(index_b);
        store_index(index_c);
      }
      break;
    }

    case vertex_property_type::uint16:
    {
      std::uint16_t index_a;
      std::uint16_t index_b;
      std::uint16_t index_c;
      load_index(index_a);
      load_index(index_b);
      if (flip_indices)
        std::swap(index_a, index_b);
      for (std::size_t index = 2; index < num_face_indices; ++index)
      {
        load_index(index_c);
        store_index(index_a);
        store_index(index_b);
        store_index(index_c);
      }
      break;
    }

    case vertex_property_type::uint32:
    {
      std::uint32_t index_a;
      std::uint32_t index_b;
      std::uint32_t index_c;
      load_index(index_a);
      load_index(index_b);
      if (flip_indices)
        std::swap(index_a, index_b);
      for (std::size_t index = 2; index < num_face_indices; ++index)
      {
        load_index(index_c);
        store_index(index_a);
        store_index(index_b);
        store_index(index_c);
      }
      break;
    }

    default:
      BOOST_ASSERT(false);
      return false;
    }
  }
  sub_mesh.index_count = index_buffer->storage.size() / index_element_size;
  mesh->sub_meshes.push_back(std::move(sub_mesh));
  mesh->index_buffer_view.size = index_buffer->storage.size();

  optimize_mesh(*mesh);

  for (auto& attribute : mesh->vertex_attributes)
    attribute.vertex_buffer_view.buffer.update_id();
  mesh->index_buffer_view.buffer.update_id();

  // Store resource into repository.
  compiler.save(*vertex_buffer, job.output_file_path("vertex-buffer", {}), job);
  compiler.save(*index_buffer, job.output_file_path("index-buffer", {}), job);
  auto mesh_filename = job.output_file_path("mesh", {});
  input.file->alias = compiler.save(*mesh, mesh_filename, job);

  return true;
}
}
