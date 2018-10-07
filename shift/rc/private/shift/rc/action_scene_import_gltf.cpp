#include "shift/rc/action_scene_import_gltf.h"
#include "shift/rc/resource_compiler_impl.h"
#include <shift/resource/buffer.h>
#include <shift/resource/mesh.h>
#include <shift/resource/scene.h>
#include <shift/parser/json/json.h>
#include <shift/math/quaternion.h>
#include <shift/math/matrix.h>
#include <shift/log/log.h>
#include <gsl/gsl>
#include <unordered_map>
#include <fstream>

namespace shift::rc
{
namespace fs = boost::filesystem;
namespace json = parser::json;

struct action_scene_import_gltf::context_t
{
  resource::scene scene;
};

action_scene_import_gltf::action_scene_import_gltf()
: action_base(action_name, action_version, support_multithreading)
{
}

action_scene_import_gltf::~action_scene_import_gltf() = default;

bool action_scene_import_gltf::process(resource_compiler_impl& /*compiler*/,
                                       job_description& job) const
{
  if (job.inputs.size() != 1)
  {
    log::error() << "The " << action_scene_import_gltf::action_name
                 << " action can only process one input at a time.";
    return false;
  }

  const input_match* scene_input = nullptr;
  for (const auto& input : job.inputs)
  {
    if (input->slot->first == "scene")
      scene_input = input.get();
  }
  if (!scene_input)
  {
    log::error() << "The " << action_scene_import_gltf::action_name
                 << " action requires an input named \"scene\".";
    return false;
  }

  const auto& scene_filename = scene_input->file->generic_string;
  std::ifstream file{scene_filename, std::ios_base::in | std::ios_base::binary};
  if (!file.is_open())
  {
    log::error() << "Failed to open file \"" << scene_filename << "\".";
    return false;
  }

  context_t context;

  json::value root;
  try
  {
    file >> root;
  }
  catch (json::parse_error&)
  {
    log::error() << "Failed to parse JSON file \"" << scene_filename << "\".";
    return false;
  }
  json::object& root_object = json::get<json::object>(root);

  const auto& asset_object = json::get<json::object>(root_object, "asset");
  const auto& version = json::get<std::string>(asset_object, "version");
  log::debug() << "Scene \"" << scene_filename << "\" is of version "
               << version;

  const auto& nodes_array = json::get<json::array>(root_object, "nodes");
  // First create all node objects so we can resolve references in one pass.
  for (auto nodes_count = nodes_array.size(); nodes_count > 0; --nodes_count)
    context.scene.nodes.emplace_back(std::make_unique<resource::scene_node>());
  std::size_t node_index = 0;
  for (const auto& node_value : nodes_array)
  {
    const auto& node_object = json::get<json::object>(node_value);
    auto* node = context.scene.nodes[node_index++].get();

    if (const auto* transformation_array =
          json::get_if<json::array>(node_object, "matrix"))
    {
      if (transformation_array->size() != 16)
      {
        BOOST_THROW_EXCEPTION(
          gltf_error{} << core::context_info(
            "A transformation matrix needs to have exactly 4x4 components."));
      }
      std::size_t index = 0;
      for (const auto& value : *transformation_array)
      {
        node->transform(index++) = static_cast<float>(json::get<double>(value));
      }
    }
    else if (json::has(node_object, "translation") ||
             json::has(node_object, "rotation") ||
             json::has(node_object, "scale"))
    {
      auto translation = math::make_identity_matrix<4, 4, float>();
      auto rotation = math::make_identity_quaternion<float>();
      auto scale = math::make_vector_from(0.0f, 0.0f, 0.0f);

      if (const auto* translation_array =
            json::get_if<json::array>(node_object, "translation"))
      {
        if (translation_array->size() != 16)
        {
          BOOST_THROW_EXCEPTION(
            gltf_error{} << core::context_info(
              "A translation matrix needs to have exactly 4x4 components."));
        }
        std::size_t index = 0;
        for (const auto& value : *translation_array)
          translation(index++) = static_cast<float>(json::get<double>(value));
      }

      if (const auto* rotation_array =
            json::get_if<json::array>(node_object, "rotation"))
      {
        if (rotation_array->size() != 4)
        {
          BOOST_THROW_EXCEPTION(
            gltf_error{} << core::context_info(
              "A rotation quaternion needs to have exactly 2x2 components."));
        }
        std::size_t index = 0;
        for (const auto& value : *rotation_array)
          rotation(index++) = static_cast<float>(json::get<double>(value));
      }

      if (const auto* scale_array =
            json::get_if<json::array>(node_object, "scale"))
      {
        if (scale_array->size() != 3)
        {
          BOOST_THROW_EXCEPTION(
            gltf_error{} << core::context_info(
              "A scale vector needs to have exactly 3 components."));
        }
        std::size_t index = 0;
        for (const auto& value : *scale_array)
          scale(index++) = static_cast<float>(json::get<double>(value));
      }

      node->transform = translation *
                        math::make_rotation_matrix<4, 4, float>(rotation) *
                        math::make_scale_matrix<4, 4, 3, float>(scale);
    }
    else
      node->transform = math::make_identity_matrix<4, 4, float>();

    // Resolve references to child nodes.
    if (const auto* children_array =
          json::get_if<json::array>(node_object, "children"))
    {
      for (const auto& value : *children_array)
      {
        auto child_index = static_cast<std::uint32_t>(json::get<double>(value));
        if (child_index >= context.scene.nodes.size())
        {
          BOOST_THROW_EXCEPTION(gltf_error{} << core::context_info(
                                  "Scene node index out of bounds."));
        }
        node->children.push_back(context.scene.nodes[child_index].get());
      }
    }
  }

  // Read the default scene index and default to the first scene if there is no
  // default defined.
  std::int32_t default_scene_index = 0;
  if (const auto* value = json::get_if<double>(root_object, "scene"))
    default_scene_index = static_cast<std::int32_t>(*value);
  for (const auto& scene_value : json::get<json::array>(root_object, "scenes"))
  {
    const auto& scene_object = json::get<json::object>(scene_value);

    // Skip scenes until we found the default one.
    if (default_scene_index-- > 0)
      continue;

    auto root_node = std::make_unique<resource::scene_node>();
    for (const auto& value : json::get<json::array>(scene_object, "nodes"))
    {
      auto node_index = static_cast<std::uint32_t>(json::get<double>(value));
      if (node_index >= context.scene.nodes.size())
      {
        BOOST_THROW_EXCEPTION(gltf_error{} << core::context_info(
                                "Scene node index out of bounds."));
      }
      root_node->children.push_back(context.scene.nodes[node_index].get());
    }

    context.scene.root = root_node.get();
    context.scene.nodes.emplace_back(std::move(root_node));
  }
  if (default_scene_index >= 0)
  {
    log::error() << "Could not locate default scene.";
  }

  return false;
}
}
