#include "shift/rc/importer_gltf/gltf.h"
#include <shift/parser/json/json.h>

namespace shift::rc::gltf
{
gltf_root::gltf_root(const parser::json::object& json_object)
: property(json_object)
{
  using std::get;

  for (const auto& child_iter : json_object)
  {
    if (child_iter.first == "accessors")
    {
      for (const auto& accessor_object :
           get<parser::json::object>(child_iter.second))
      {
        accessors.insert(
          std::make_pair(accessor_object.first,
                         std::make_unique<gltf::accessor>(
                           get<parser::json::object>(accessor_object.second))));
      }
    }
    else if (child_iter.first == "bufferViews")
    {
      for (const auto& buffer_view_object :
           get<parser::json::object>(child_iter.second))
      {
        buffer_views.insert(std::make_pair(
          buffer_view_object.first,
          std::make_unique<gltf::buffer_view>(
            get<parser::json::object>(buffer_view_object.second))));
      }
    }
    else if (child_iter.first == "buffers")
    {
      for (const auto& buffer_object :
           get<parser::json::object>(child_iter.second))
      {
        buffers.insert(
          std::make_pair(buffer_object.first,
                         std::make_unique<gltf::buffer>(
                           get<parser::json::object>(buffer_object.second))));
      }
    }
    else if (child_iter.first == "materials")
    {
      for (const auto& material_object :
           get<parser::json::object>(child_iter.second))
      {
        materials.insert(
          std::make_pair(material_object.first,
                         std::make_unique<gltf::material>(
                           get<parser::json::object>(material_object.second))));
      }
    }
    else if (child_iter.first == "meshes")
    {
      for (const auto& mesh_object :
           get<parser::json::object>(child_iter.second))
      {
        meshes.insert(std::make_pair(
          mesh_object.first, std::make_unique<gltf::mesh>(
                               get<parser::json::object>(mesh_object.second))));
      }
    }
  }

  // Resolve all objects referenced by IDs.

  for (auto& buffer_view : buffer_views)
  {
    auto buffer_iter = buffers.find(buffer_view.second->buffer_id);
    if (buffer_iter != buffers.end())
      buffer_view.second->buffer = buffer_iter->second.get();
    else
    {
      BOOST_THROW_EXCEPTION(
        parse_error() << parse_error_info(
          "Cannot resolve reference to non-existing buffer '" +
          buffer_view.second->buffer_id + "'."));
    }
  }

  for (auto& accessor : accessors)
  {
    auto buffer_view_iter = buffer_views.find(accessor.second->buffer_view_id);
    if (buffer_view_iter != buffer_views.end())
      accessor.second->buffer_view = buffer_view_iter->second.get();
    else
    {
      BOOST_THROW_EXCEPTION(
        parse_error() << parse_error_info(
          "Cannot resolve reference to non-existing buffer view '" +
          accessor.second->buffer_view_id + "'."));
    }
  }

  for (auto& mesh : meshes)
  {
    for (auto& primitive : mesh.second->primitives)
    {
      if (!primitive->indices_id.empty())
      {
        auto accessor_iter = accessors.find(primitive->indices_id);
        if (accessor_iter != accessors.end())
          primitive->indices = accessor_iter->second.get();
        else
        {
          BOOST_THROW_EXCEPTION(
            parse_error() << parse_error_info(
              "Cannot resolve reference to non-existing accessor '" +
              primitive->indices_id + "'."));
        }
      }

      auto material_iter = materials.find(primitive->material_id);
      if (material_iter != materials.end())
        primitive->material = material_iter->second.get();
      else
      {
        BOOST_THROW_EXCEPTION(
          parse_error() << parse_error_info(
            "Cannot resolve reference to non-existing material '" +
            primitive->indices_id + "'."));
      }
    }
  }
}
}
