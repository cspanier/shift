#include "shift/rc/action_mesh_export_obj.hpp"
#include "shift/rc/resource_compiler_impl.hpp"
#include <shift/resource_db/buffer.hpp>
#include <shift/resource_db/mesh.hpp>
#include <shift/log/log.hpp>
#include <boost/filesystem/operations.hpp>
#include <fstream>
#include <unordered_map>

namespace std
{
template <>
struct hash<shift::math::vector4<float>>
{
  std::size_t operator()(const shift::math::vector4<float>& v) const
  {
    std::size_t result = std::hash<float>{}(v.x);
    boost::hash_combine(result, std::hash<float>{}(v.y));
    boost::hash_combine(result, std::hash<float>{}(v.z));
    boost::hash_combine(result, std::hash<float>{}(v.w));
    return result;
  }
};
}

namespace shift::rc
{
namespace fs = boost::filesystem;

action_mesh_export_obj::action_mesh_export_obj()
: action_base(action_name, action_version)
{
}

bool action_mesh_export_obj::process(resource_compiler_impl& compiler,
                                     job_description& job) const
{
  if (job.inputs.size() != 1)
  {
    log::error() << "The " << action_mesh_export_obj::action_name
                 << " action can only process one input at a time.";
    return false;
  }

  const input_match* mesh_input = nullptr;
  for (const auto& [input_slot_index, input] : job.inputs)
  {
    if (input->slot->first == "mesh")
      mesh_input = input.get();
  }
  if (!mesh_input)
  {
    log::error() << "The " << action_mesh_export_obj::action_name
                 << " action requires an input named \"mesh\".";
    return false;
  }

  // Load mesh and check if all referenced buffers exist.
  auto& repository = resource_db::repository::singleton_instance();
  auto mesh = repository.load<resource_db::mesh>(mesh_input->file->path);
  if (!mesh)
  {
    log::error() << "Cannot load mesh " << mesh_input->file->path << ".";
    return false;
  }

  resource_db::vertex_attribute* position_attribute = nullptr;
  resource_db::vertex_attribute* normal_attribute = nullptr;
  resource_db::vertex_attribute* texcoord_attribute = nullptr;
  for (auto& vertex_attribute : mesh->vertex_attributes)
  {
    if (!vertex_attribute.vertex_buffer_view.buffer.get_shared())
    {
      log::error() << "Cannot load a vertex buffer referenced in mesh "
                   << mesh_input->file->path << ".";
      return false;
    }
    if (vertex_attribute.usage == resource_db::vertex_attribute_usage::position)
    {
      if (position_attribute)
      {
        log::error() << "A mesh cannot have two position attributes.";
        return false;
      }
      position_attribute = &vertex_attribute;
    }
    else if (vertex_attribute.usage == resource_db::vertex_attribute_usage::normal)
    {
      if (normal_attribute)
      {
        log::error() << "A mesh cannot have two normal attributes.";
        return false;
      }
      normal_attribute = &vertex_attribute;
    }
    else if (vertex_attribute.usage ==
             resource_db::vertex_attribute_usage::texcoord)
    {
      if (texcoord_attribute)
      {
        log::error() << "A mesh cannot have two texture coordinate attributes.";
        return false;
      }
      texcoord_attribute = &vertex_attribute;
    }
    else
      log::warning() << "Skipping unsupported vertex attribute.";
  }

  auto index_buffer = mesh->index_buffer_view.buffer.get_shared();
  if (!index_buffer)
  {
    log::error() << "Cannot load index buffer referenced in mesh "
                 << mesh_input->file->path << ".";
    return false;
  }

  auto obj_filename = job.output_file_path("mesh", {});
  fs::create_directories(obj_filename.parent_path());
  std::ofstream obj_file(
    obj_filename.generic_string(),
    std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  if (!obj_file.is_open())
  {
    log::error() << "Cannot write output file " << obj_filename << ".";
    return false;
  }

  std::unordered_map<math::vector4<float>, std::uint32_t> positions;
  std::unordered_map<math::vector4<float>, std::uint32_t> normals;
  std::unordered_map<math::vector4<float>, std::uint32_t> texcoords;
  std::vector<std::uint32_t> position_indices;
  std::vector<std::uint32_t> normal_indices;
  std::vector<std::uint32_t> texcoord_indices;

  // Export vertex positions.
  if (position_attribute)
  {
    const auto& buffer_view = position_attribute->vertex_buffer_view;
    const auto& buffer = *buffer_view.buffer.get_shared();
    if (position_attribute->component_type !=
        resource_db::vertex_attribute_component_type::float32)
    {
      log::error() << "Unsupported position component type.";
      return false;
    }
    std::size_t offset = position_attribute->offset;
    std::size_t stride = position_attribute->stride;
    std::uint32_t next_index = 0;
    switch (position_attribute->data_type)
    {
    case resource_db::vertex_attribute_data_type::vec3:
      if (!stride)
        stride = 3 * sizeof(float);
      for (std::size_t i = 0; i < buffer.storage.size() / stride; ++i)
      {
        const auto* position_data =
          reinterpret_cast<const float*>(&buffer.storage[i * stride + offset]);
        math::vector4<float> position{position_data[0], position_data[1],
                                      position_data[2], 1.0f};

        decltype(positions)::iterator position_iter;
        bool unique = false;
        std::tie(position_iter, unique) =
          positions.insert(std::make_pair(position, next_index));
        if (unique)
        {
          ++next_index;
          obj_file << "v " << position_data[0] << " " << position_data[1] << " "
                   << position_data[2] << "\n";
        }
        position_indices.push_back(position_iter->second);
      }
      break;

    case resource_db::vertex_attribute_data_type::vec4:
      if (!stride)
        stride = 4 * sizeof(float);
      for (std::size_t i = 0; i < buffer.storage.size() / stride; ++i)
      {
        const auto* position_data =
          reinterpret_cast<const float*>(&buffer.storage[i * stride + offset]);
        math::vector4<float> position{position_data[0], position_data[1],
                                      position_data[2], position_data[3]};

        decltype(positions)::iterator position_iter;
        bool unique = false;
        std::tie(position_iter, unique) =
          positions.insert(std::make_pair(position, next_index));
        if (unique)
        {
          ++next_index;
          obj_file << "v " << position_data[0] << " " << position_data[1] << " "
                   << position_data[2] << " " << position_data[3] << "\n";
        }
        position_indices.push_back(position_iter->second);
      }
      break;

    default:
      log::error() << "Unsupported position data type.";
      return false;
    }
  }
  else
  {
    log::error() << "A mesh is required to have a vertex position attribute.";
    return false;
  }

  // Export vertex texture coordinates.
  if (texcoord_attribute)
  {
    const auto& buffer_view = texcoord_attribute->vertex_buffer_view;
    const auto& buffer = *buffer_view.buffer.get_shared();
    if (texcoord_attribute->component_type !=
        resource_db::vertex_attribute_component_type::float32)
    {
      log::error() << "Unsupported texcoord component type.";
      return false;
    }
    std::size_t offset = texcoord_attribute->offset;
    std::size_t stride = texcoord_attribute->stride;
    std::uint32_t next_index = 0;
    switch (texcoord_attribute->data_type)
    {
    case resource_db::vertex_attribute_data_type::vec2:
      if (!stride)
        stride = 2 * sizeof(float);
      for (std::size_t i = 0; i < buffer.storage.size() / stride; ++i)
      {
        const auto* texcoord_data =
          reinterpret_cast<const float*>(&buffer.storage[i * stride + offset]);
        math::vector4<float> texcoord{texcoord_data[0], texcoord_data[1], 0.0f,
                                      0.0f};

        decltype(texcoords)::iterator texcoord_iter;
        bool unique = false;
        std::tie(texcoord_iter, unique) =
          texcoords.insert(std::make_pair(texcoord, next_index));
        if (unique)
        {
          ++next_index;
          obj_file << "vt " << texcoord_data[0] << " " << texcoord_data[1]
                   << "\n";
        }
        texcoord_indices.push_back(texcoord_iter->second);
      }
      break;

    default:
      log::error() << "Unsupported texcoord data type.";
      return false;
    }
  }
  else
  {
    log::error() << "A mesh is required to have a vertex texcoord attribute.";
    return false;
  }
  // Export vertex normals.
  if (normal_attribute)
  {
    const auto& buffer_view = normal_attribute->vertex_buffer_view;
    const auto& buffer = *buffer_view.buffer.get_shared();
    if (normal_attribute->component_type !=
        resource_db::vertex_attribute_component_type::float32)
    {
      log::error() << "Unsupported normal component type.";
      return false;
    }
    std::size_t offset = normal_attribute->offset;
    std::size_t stride = normal_attribute->stride;
    std::uint32_t next_index = 0;
    switch (normal_attribute->data_type)
    {
    case resource_db::vertex_attribute_data_type::vec3:
      if (!stride)
        stride = 3 * sizeof(float);
      for (std::size_t i = 0; i < buffer.storage.size() / stride; ++i)
      {
        const auto* normal_data =
          reinterpret_cast<const float*>(&buffer.storage[i * stride + offset]);
        math::vector4<float> normal{normal_data[0], normal_data[1],
                                    normal_data[2], 0.0f};

        decltype(normals)::iterator normal_iter;
        bool unique = false;
        std::tie(normal_iter, unique) =
          normals.insert(std::make_pair(normal, next_index));
        if (unique)
        {
          ++next_index;
          obj_file << "vn " << normal_data[0] << " " << normal_data[1] << " "
                   << normal_data[2] << "\n";
        }
        normal_indices.push_back(normal_iter->second);
      }
      break;

    default:
      log::error() << "Unsupported normal data type.";
      return false;
    }
  }
  else
  {
    log::error() << "A mesh is required to have a vertex normal attribute.";
    return false;
  }

  // Export polygon indices.
  for (const auto& sub_mesh : mesh->sub_meshes)
  {
    const auto& buffer = *index_buffer.get();
    auto fetch_position_index = [&](std::uint32_t i) {
      return position_indices[*(reinterpret_cast<const std::uint32_t*>(
                                  buffer.storage.data()) +
                                (i + sub_mesh.first_index)) +
                              sub_mesh.vertex_offset] +
             1;
    };
    auto fetch_texcoord_index = [&](std::uint32_t i) {
      return texcoord_indices[*(reinterpret_cast<const std::uint32_t*>(
                                  buffer.storage.data()) +
                                (i + sub_mesh.first_index)) +
                              sub_mesh.vertex_offset] +
             1;
    };
    auto fetch_normal_index = [&](std::uint32_t i) {
      return normal_indices[*(reinterpret_cast<const std::uint32_t*>(
                                buffer.storage.data()) +
                              (i + sub_mesh.first_index)) +
                            sub_mesh.vertex_offset] +
             1;
    };

    switch (sub_mesh.topology)
    {
    case resource_db::primitive_topology::triangle_list:
      for (std::uint32_t i = 0; i < sub_mesh.index_count; i += 3)
      {
        obj_file << "f " << fetch_position_index(i + 0) << "/"
                 << fetch_texcoord_index(i + 0) << "/"
                 << fetch_normal_index(i + 0) << " "
                 << fetch_position_index(i + 1) << "/"
                 << fetch_texcoord_index(i + 1) << "/"
                 << fetch_normal_index(i + 1) << " "
                 << fetch_position_index(i + 2) << "/"
                 << fetch_texcoord_index(i + 2) << "/"
                 << fetch_normal_index(i + 2) << "\n";
      }
      break;

    default:
      log::error() << "Unsupported primitive topology.";
      return false;
    }
  }
  obj_file.close();

  mesh_input->file->alias = compiler.push(obj_filename, job);
  return true;
}
}
