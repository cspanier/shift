#include "shift/rc/action_scene_import_pbrt.h"
#include "shift/rc/resource_compiler_impl.h"
#include <shift/resource/scene.h>
#include <shift/log/log.h>
#include <shift/math/matrix.h>
#include <shift/math/vector.h>
#include <shift/core/string_util.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <cctype>

namespace shift::rc
{
using namespace std::string_literals;
namespace fs = boost::filesystem;

enum class texture_format
{
  undefined,
  floating_point,
  color,
  spectrum
};

enum class texture_sampler
{
  undefined,
  imagemap,
  constant,
  scale
};

struct texture_description
{
  fs::path filename;
  // texture_mapping mapping = texture_mapping::undefined;
  resource::sampler_address_mode wrap = resource::sampler_address_mode::repeat;
  math::vector2<float> scale = math::vector2<float>(1.0f, 1.0f);
  math::vector2<float> offset = math::vector2<float>(0.0f, 0.0f);
  float max_anisotropy = 1.0f;
  texture_format format = texture_format::undefined;
  texture_sampler sampler = texture_sampler::undefined;
};

struct parser_scope
{
  std::string object_name;
  std::map<std::string, std::shared_ptr<texture_description>> named_textures;
  std::map<std::string, std::shared_ptr<resource::material>> named_materials;
};

struct action_scene_import_pbrt::parser_context
{
  parser_context(resource_compiler_impl& compiler, job_description& job)
  : compiler(compiler), job(job)
  {
  }

  parser_context(const parser_context&) = default;
  parser_context(parser_context&&) = default;
  ~parser_context() = default;
  parser_context& operator=(const parser_context&) = default;
  parser_context& operator=(parser_context&&) = default;

  resource_compiler_impl& compiler;
  job_description& job;
  resource::scene scene;
  std::stack<std::unique_ptr<resource::scene_node>> current_nodes;
  std::map<std::string, resource::scene_node*> named_nodes;

  std::stack<parser_scope> scope;
  std::vector<std::shared_ptr<texture_description>> textures;
  std::map<math::vector4<float>, resource::resource_ptr<resource::image>>
    color_images;
};

action_scene_import_pbrt::action_scene_import_pbrt()
: action_base(action_name, action_version)
{
}

bool action_scene_import_pbrt::process(resource_compiler_impl& compiler,
                                       job_description& job) const
{
  if (job.inputs.size() != 1)
  {
    log::error() << "The " << action_scene_import_pbrt::action_name
                 << " action can only process one input at a time.";
    return false;
  }
  const auto& input = job.inputs.at(0);

  if (!fs::exists(input->file->path) || !fs::is_regular_file(input->file->path))
  {
    log::error() << "Cannot find input file " << input->file->path << ".";
    return false;
  }

  // PBRT files reference other files relative to the original source file's
  // location, which we call include_folder.
  auto include_path = input->file->path;
  include_path.remove_filename();

  parser_context context{compiler, job};
  context.scope.push({});
  /// ToDo: Does input->file->path.root_path() work as base_path?
  if (!parse_file(context, input->file->path.root_path(), include_path,
                  input->file->path))
  {
    return false;
  }
  context.scope.pop();
  BOOST_ASSERT(context.scope.empty());

  for (auto& node : context.scene.nodes)
  {
    if (!node->mesh || !node->material)
    {
      node->mesh.reset();
      node->material.reset();
    }
  }

  // Store resource into repository.
  auto scene_filename = job.output("scene", {});
  input->file->alias = compiler.save(context.scene, scene_filename, job);

  return true;
}

bool action_scene_import_pbrt::parse_token_group(
  action_scene_import_pbrt::parser_context& context,
  const std::vector<std::string>& tokens, const fs::path& base_path,
  const fs::path& include_path) const
{
  auto& repository = resource::repository::singleton_instance();
  auto token_iter = tokens.begin();
  auto has_tokens = [&]() { return token_iter != tokens.end(); };
  auto get_token = [&]() {
    if (token_iter == tokens.end())
    {
      BOOST_THROW_EXCEPTION(pbrt_error()
                            << core::context_info("Expected token."));
    }
    return *(token_iter++);
  };
  auto split_type = [](const std::string& token) {
    auto splitter = std::find(token.begin(), token.end(), ' ');
    if (splitter == token.end())
    {
      log::warning() << "Expected type and name token, but got \"" << token
                     << "\".";
      return std::make_pair(""s, token);
    }
    else
    {
      return std::make_pair(std::string(token.begin(), splitter),
                            std::string(splitter + 1, token.end()));
    }
  };
  auto skip_argument = [&]() {
    if (has_tokens() && get_token() == "[")
    {
      do
      {
      } while (has_tokens() && get_token() != "]");
    }
  };
  auto get_string = [&]() {
    auto token = get_token();
    bool is_array = false;
    if (token == "[")
    {
      is_array = true;
      token = get_token();
    }
    if (is_array)
    {
      if (get_token() != "]")
      {
        BOOST_THROW_EXCEPTION(pbrt_error()
                              << core::context_info("Expected ']'."));
      }
    }
    return token;
  };
  auto get_float = [&]() {
    auto token = get_token();
    bool is_array = false;
    if (token == "[")
    {
      is_array = true;
      token = get_token();
    }
    if (is_array)
    {
      if (get_token() != "]")
      {
        BOOST_THROW_EXCEPTION(pbrt_error()
                              << core::context_info("Expected ']'."));
      }
    }
    return boost::lexical_cast<float>(token);
  };
  auto get_color = [&]() {
    auto color = math::make_vector_from(0.0f, 0.0f, 0.0f, 1.0f);
    auto token = get_token();
    bool is_array = false;
    if (token == "[")
    {
      is_array = true;
      token = get_token();
    }
    color.x = boost::lexical_cast<float>(token);
    token = get_token();
    if (is_array && token == "]")
    {
      color.y = color.x;
      color.z = color.x;
    }
    else
    {
      color.y = boost::lexical_cast<float>(token);
      token = get_token();
      color.z = boost::lexical_cast<float>(token);
      token = get_token();
      if (is_array && token != "]")
      {
        color.w = boost::lexical_cast<float>(token);
        token = get_token();
      }
      if (is_array && token != "]")
      {
        BOOST_THROW_EXCEPTION(pbrt_error()
                              << core::context_info("Expected ']'."));
      }
    }
    return color;
  };

  auto parse_texture = [&](texture_description& texture) {
    auto texture_format = get_token();
    if (texture_format == "float")
      texture.format = texture_format::floating_point;
    else if (texture_format == "color")
      texture.format = texture_format::color;
    else if (texture_format == "spectrum")
      texture.format = texture_format::spectrum;
    else
    {
      log::warning() << "Ignoring unknown texture format \"" << texture_format
                     << "\".";
      return false;
    }

    auto texture_sampler = get_token();
    if (texture_sampler == "imagemap")
    {
      texture.sampler = texture_sampler::imagemap;
    }
    else if (texture_sampler == "constant")
    {
      texture.sampler = texture_sampler::constant;
    }
    else if (texture_sampler == "scale")
    {
      // Texture "Map #595_bump" "float" "scale"
      // "texture tex1" "Map #595"
      // "float tex2" [0.01]

      texture.sampler = texture_sampler::scale;
    }
    else
    {
      log::warning() << "Ignoring unknown texture sampler \"" << texture_sampler
                     << "\".";
      return false;
    }

    while (has_tokens())
    {
      auto token = get_token();
      std::string argument_type;
      std::string argument_name;
      std::tie(argument_type, argument_name) = split_type(token);
      if (argument_name == "filename")
      {
        if (argument_type == "string")
          texture.filename = include_path / get_string();
        else
        {
          log::warning() << "Unsupported argument type \"" << argument_type
                         << "\" for texture argument \"" << argument_name
                         << "\".";
          return false;
        }
      }
      else if (argument_name == "mapping")
      {
        if (argument_type == "string")
        {
          auto mapping = get_string();
          if (mapping != "uv")
          {
            log::warning() << "Unsupported texture mapping \"" << mapping
                           << "\".";
            return false;
          }
        }
        else
        {
          log::warning() << "Unsupported argument type \"" << argument_type
                         << "\" for texture argument \"" << argument_name
                         << "\".";
          return false;
        }
      }
      else if (argument_name == "wrap")
      {
        if (argument_type == "string")
        {
          auto wrapping = get_string();
          if (wrapping == "repeat")
            texture.wrap = resource::sampler_address_mode::repeat;
          else
          {
            log::warning() << "Unsupported texture wrapping \"" << wrapping
                           << "\".";
            return false;
          }
        }
        else
        {
          log::warning() << "Unsupported argument type \"" << argument_type
                         << "\" for texture argument \"" << argument_name
                         << "\".";
          return false;
        }
      }
      else if (argument_name == "uscale")
      {
        if (argument_type == "float")
          texture.scale.x = get_float();
        else
        {
          log::warning() << "Unsupported argument type \"" << argument_type
                         << "\" for texture argument \"" << argument_name
                         << "\".";
          return false;
        }
      }
      else if (argument_name == "vscale")
      {
        if (argument_type == "float")
          texture.scale.y = get_float();
        else
        {
          log::warning() << "Unsupported argument type \"" << argument_type
                         << "\" for texture argument \"" << argument_name
                         << "\".";
          return false;
        }
      }
      else if (argument_name == "udelta")
      {
        if (argument_type == "float")
          texture.offset.x = get_float();
        else
        {
          log::warning() << "Unsupported argument type \"" << argument_type
                         << "\" for texture argument \"" << argument_name
                         << "\".";
          return false;
        }
      }
      else if (argument_name == "vdelta")
      {
        if (argument_type == "float")
          texture.offset.y = get_float();
        else
        {
          log::warning() << "Unsupported argument type \"" << argument_type
                         << "\" for texture argument \"" << argument_name
                         << "\".";
          return false;
        }
      }
      else if (argument_name == "trilinear" ||
               argument_name == "maxanisotropy" || argument_name == "value" ||
               argument_name == "tex1" || argument_name == "tex2")
      {
        skip_argument();
      }
      else
      {
        log::warning() << "Ignoring unknown texture argument \""
                       << argument_name << "\".";
        skip_argument();
      }
    }

    return !texture.filename.empty();
  };

  auto parse_material = [&](resource::material& material,
                            std::string type_name) {
    auto& repository = resource::repository::singleton_instance();

    // Creates a 1x1 texture containing a single pixel of the requested color.
    auto assign_color =
      [&](math::vector4<float> color,
          std::pair<resource::image_reference, resource::sampler>& image_map) {
        auto& image_reference = image_map.first;
        auto& sampler = image_map.second;

        auto color_image_iter = context.color_images.find(color);
        if (color_image_iter == context.color_images.end())
        {
          image_reference.image = std::make_shared<resource::image>();
          image_reference.image->format =
            resource::image_format::r32g32b32a32_sfloat;
          image_reference.image->array_element_count = 1;
          image_reference.image->face_count = 1;

          {
            resource::mipmap_info mipmap;
            mipmap.width = 1;
            mipmap.height = 1;
            mipmap.depth = 1;
            mipmap.buffer = std::make_shared<resource::buffer>();
            mipmap.buffer->storage.resize(sizeof(color));
            std::memcpy(mipmap.buffer->storage.data(), &color, sizeof(color));
            mipmap.offset = 0;
            image_reference.image->mipmaps.push_back(std::move(mipmap));
          }

          std::stringstream color_name;
          for (auto i = 0u; i < color.row_count; ++i)
          {
            color_name << std::hex << std::setfill('0') << std::setw(4)
                       << core::clamp(
                            static_cast<std::int32_t>(color(i) * 65535), 0,
                            65535);
          }

          auto image_buffer_filename = context.job.output(
            "material-image-buffer",
            {std::make_pair("image-color", color_name.str())});
          for (auto& mipmap : image_reference.image->mipmaps)
            mipmap.buffer.update_id();
          context.compiler.save(*image_reference.image->mipmaps.front().buffer,
                                image_buffer_filename, context.job);

          auto image_header_filename = context.job.output(
            "material-image-header",
            {std::make_pair("image-color", color_name.str())});
          image_reference.image.update_id();
          context.compiler.save(*image_reference.image, image_header_filename,
                                context.job);

          context.color_images.insert_or_assign(
            color, image_reference.image.get_shared());
        }
        else
          image_reference.image = color_image_iter->second;

        image_reference.offset = math::make_vector_from(0.0f, 0.0f);
        image_reference.scale = math::make_vector_from(1.0f, 1.0f);
        sampler.address_mode_u = resource::sampler_address_mode::repeat;
        sampler.address_mode_v = resource::sampler_address_mode::repeat;
        sampler.address_mode_w = resource::sampler_address_mode::repeat;
        sampler.max_anisotropy = 1.0f;
        sampler.min_lod = 0.0f;
        sampler.max_lod = 0.25f;
      };

    auto assign_texture =
      [&](const std::string& texture_name,
          std::pair<resource::image_reference, resource::sampler>& image_map) {
        auto& image_reference = image_map.first;
        auto& sampler = image_map.second;

        resource::resource_ptr<resource::image> image;
        std::shared_ptr<texture_description> texture;
        if (!texture_name.empty())
        {
          auto& current_scope = context.scope.top();
          auto texture_iter = current_scope.named_textures.find(texture_name);
          if (texture_iter != current_scope.named_textures.end())
          {
            texture = texture_iter->second;
            auto current_pass = context.job.rule->pass;
            auto* texture_file = context.compiler.alias(
              context.compiler.get_file(texture->filename), current_pass);
            if (texture_file)
              image = repository.load<resource::image>(texture_file->path);
          }
          else
          {
            log::warning() << "Cannot find referenced texture \""
                           << texture_name << "\".";
          }
        }

        if (!image)
          return;

        image_reference.image = image;
        image_reference.image.update_id();
        if (texture)
        {
          image_reference.scale = texture->scale;
          image_reference.offset = texture->offset;
          sampler.address_mode_u = texture->wrap;
          sampler.address_mode_v = texture->wrap;
          sampler.address_mode_w = texture->wrap;
          sampler.max_anisotropy = texture->max_anisotropy;
        }
        sampler.min_lod = 0.0f;
        sampler.max_lod = image->mipmaps.size();
      };

    auto assign_replaceme =
      [&](std::pair<resource::image_reference, resource::sampler>& image_map) {
        auto& image_reference = image_map.first;
        auto& sampler = image_map.second;

        if (image_reference.image)
          return true;

        auto current_pass = context.job.rule->pass;
        auto* image_file = context.compiler.alias(
          context.compiler.get_file(
            fs::path{"private/textures/engine/replaceme.png"}),
          current_pass);
        if (!image_file)
          return false;
        /// ToDo: Pass the exact path using rule options!
        auto image = repository.load<resource::image>(image_file->path);
        if (!image)
        {
          log::error() << "Cannot find \"replaceme.png\" dummy texture.";
          return false;
        }

        image_reference.image = image;
        sampler.min_lod = 0.0f;
        sampler.max_lod = image->mipmaps.size();
        return true;
      };

    while (has_tokens())
    {
      auto token = get_token();
      std::string argument_type;
      std::string argument_name;
      std::tie(argument_type, argument_name) = split_type(token);
      if (argument_name == "type")
      {
        if (argument_type == "string")
        {
          type_name = get_string();
          // We check type_name later because it might have been passed as an
          // argument.
        }
        else
        {
          log::warning() << "Unsupported material type \"" << argument_type
                         << "\".";
          return false;
        }
      }
      else if (argument_name == "Kd")  // diffuse/albedo
      {
        if (argument_type == "color" || argument_type == "rgb")
          assign_color(get_color(), material.albedo_map);
        else if (argument_type == "texture")
          assign_texture(get_string(), material.albedo_map);
        else
        {
          log::warning() << "Ignoring unknown argument_type \"" << argument_type
                         << R"(" for argument "Kd".)";
        }
      }
      else if (argument_name == "Ks")  // specular
      {
        if (argument_type == "color" || argument_type == "rgb")
          assign_color(get_color(), material.specular_map);
        else if (argument_type == "texture")
          assign_texture(get_string(), material.specular_map);
        else
        {
          log::warning() << "Ignoring unknown argument_type \"" << argument_type
                         << R"(" for argument "Ks".)";
        }
      }
      else if (argument_name == "Kr")  // reflection => roughness?
      {
        if (argument_type == "color" || argument_type == "rgb")
          get_color();  /// ToDo: Create 1x1 texture with specified color.
        else if (argument_type == "texture")
          get_string();
        else
        {
          log::warning() << "Ignoring unknown argument_type \"" << argument_type
                         << R"(" for argument "Kr".)";
        }
      }
      else if (argument_name == "Kt")
      {
        // The coefficient of specular transmission. Always rgb/color so far.
        if (argument_type == "color" || argument_type == "rgb")
          get_color();
        else
        {
          log::warning() << "Ignoring unknown argument_type \"" << argument_type
                         << R"(" for argument "Kt".)";
        }
      }
      else if (argument_name == "roughness")
      {
        if (argument_type == "float")
          assign_color(get_color(), material.roughness_map);
        else if (argument_type == "texture")
          assign_texture(get_string(), material.roughness_map);
        else
        {
          log::warning() << "Ignoring unknown argument_type \"" << argument_type
                         << R"(" for argument "roughness".)";
        }
      }
      else if (argument_name == "index")
      {
        if (argument_type == "float")
          get_float();
        else
        {
          log::warning() << "Ignoring unknown argument_type \"" << argument_type
                         << R"(" for argument "index".)";
        }
      }
      else if (argument_name == "sigma" || argument_name == "sigma_a" ||
               argument_name == "sigma_s" || argument_name == "bumpmap" ||
               argument_name == "opacity" || argument_name == "eta" ||
               argument_name == "k" || argument_name == "bsdffile" ||
               argument_name == "reflect" || argument_name == "transmit" ||
               argument_name == "namedmaterial1" ||
               argument_name == "namedmaterial2" || argument_name == "amount")
      {
        skip_argument();
      }
      else
      {
        log::warning() << "Ignoring unknown material argument \""
                       << argument_name << "\".";
        skip_argument();
      }
    }

    if (!assign_replaceme(material.albedo_map))
      return false;
    if (!assign_replaceme(material.ambient_occlusion_map))
      return false;
    if (!assign_replaceme(material.normal_map))
      return false;
    if (!assign_replaceme(material.height_map))
      return false;
    if (!assign_replaceme(material.roughness_map))
      return false;
    if (!assign_replaceme(material.metalness_map))
      return false;
    if (!assign_replaceme(material.specular_map))
      return false;

    // if (type_name == "uber")
    //  material.type = material_type::uber;
    // else if (type_name == "matte")
    //  material.type = material_type::matte;
    // else if (type_name == "glass")
    //  material.type = material_type::glass;
    // else if (type_name == "plastic")
    //  material.type = material_type::plastic;
    // else if (type_name == "mix")
    //  material.type = material_type::mix;
    // else if (type_name == "translucent")
    //  material.type = material_type::translucent;
    // else if (type_name == "metal")
    //  material.type = material_type::metal;
    // else if (type_name == "fourier")
    //  material.type = material_type::fourier;
    // else
    //{
    //  log::warning() << "Ignoring unknown material type \"" << type_name
    //                 << "\".";
    //  material.type = material_type::undefined;
    //}
    return true;
  };

  auto token = get_token();
  if (token == "WorldBegin")
  {
    if (context.scene.root)
    {
      log::error() << "Specifying WorldBegin more than once is not allowed.";
      return false;
    }

    auto new_node = std::make_unique<resource::scene_node>();
    context.scene.root = new_node.get();
    context.current_nodes.push(std::move(new_node));
  }
  else if (token == "WorldEnd")
  {
    if (context.current_nodes.size() != 1)
    {
      BOOST_THROW_EXCEPTION(
        pbrt_error() << core::context_info(
          "\"AttributeBegin\" and \"AttributeEnd\" tokens don't match."));
    }

    context.scene.nodes.push_back(std::move(context.current_nodes.top()));
    context.current_nodes.pop();
  }
  else if (token == "AttributeBegin")
  {
    if (context.current_nodes.empty())
    {
      log::warning() << "Ignoring \"AttributeBegin\" token outside "
                        "\"WorldBegin\"/\"WorldEnd\" token group.";
      return true;
    }
    // auto* current_node = context.current_nodes.top().get();
    context.current_nodes.push(std::make_unique<resource::scene_node>());

    context.scope.push(context.scope.top());
    context.scope.top().object_name.clear();
  }
  else if (token == "AttributeEnd")
  {
    if (context.current_nodes.empty())
    {
      BOOST_THROW_EXCEPTION(
        pbrt_error() << core::context_info(
          "\"AttributeBegin\" and \"AttributeEnd\" tokens don't match."));
    }
    auto current_node = std::move(context.current_nodes.top());
    context.current_nodes.pop();

    if (!current_node->children.empty() ||
        (current_node->mesh && current_node->material))
    {
      if (!context.scope.top().object_name.empty())
      {
        context.named_nodes[context.scope.top().object_name] =
          current_node.get();
      }
      else
        context.current_nodes.top()->children.push_back(current_node.get());
      context.scene.nodes.push_back(std::move(current_node));
    }

    BOOST_ASSERT(context.scope.size() > 1);
    if (context.scope.size() > 1)
      context.scope.pop();
  }
  else if (token == "ObjectBegin")
  {
    context.scope.top().object_name = get_token();
  }
  else if (token == "ObjectEnd")
  {
    // The object name has already been poped from the scope stack.
    // context.scope.top().object_name.clear();
  }
  else if (token == "Texture")
  {
    auto texture_name = get_token();
    auto new_texture = std::make_shared<texture_description>();

    if (parse_texture(*new_texture))
    {
      auto& current_scope = context.scope.top();
      if (!current_scope.named_textures
             .insert_or_assign(texture_name, new_texture)
             .second)
      {
        log::warning() << "Overwriting already existing texture \""
                       << texture_name << "\".";
      }
      context.textures.push_back(new_texture);

      // auto texture_filename = context.job.output(
      //  "texture", {std::make_pair("texture-name", texture_name)});
      // context.compiler.save(*new_texture, texture_filename, context.job);
    }
  }
  else if (token == "Material")
  {
    // Anonymous material.
    auto* current_node = context.current_nodes.top().get();
    if (current_node->material)
      log::warning() << "Already selected a material";

    std::string material_type = get_token();
    auto new_material = std::make_shared<resource::material>();

    if (parse_material(*new_material, material_type))
    {
      // context.materials.push_back(new_material);
      current_node->material = new_material;

      std::stringstream material_name;
      material_name << std::hex << std::setfill('0') << std::setw(16)
                    << new_material->id();
      auto material_filename = context.job.output(
        "material", {std::make_pair("material-name", material_name.str())});
      context.compiler.save(*new_material, material_filename, context.job);
    }
  }
  else if (token == "MakeNamedMaterial")
  {
    std::string material_name = get_token();
    auto& current_scope = context.scope.top();
    auto new_material = std::make_shared<resource::material>();

    if (parse_material(*new_material, "uber"))
    {
      auto material_iter = current_scope.named_materials.find(material_name);
      if (material_iter == current_scope.named_materials.end())
      {
        current_scope.named_materials.insert_or_assign(material_name,
                                                       new_material);
        auto material_filename = context.job.output(
          "material", {std::make_pair("material-name", material_name)});
        context.compiler.save(*new_material, material_filename, context.job);
        // context.materials.push_back(std::move(new_material));
      }
      else
      {
        log::warning() << "Skipping already existing material \""
                       << material_name << "\"..";
      }
    }
  }
  else if (token == "NamedMaterial")
  {
    auto& current_scope = context.scope.top();
    auto* current_node = context.current_nodes.top().get();
    if (current_node->material)
      log::warning() << "Already selected a material";

    std::string material_name = get_token();
    auto material_iter = current_scope.named_materials.find(material_name);
    if (material_iter == current_scope.named_materials.end())
    {
      log::warning() << "Cannot find named material \"" << material_name
                     << "\".";
      // if (!context.materials.empty())
      //{
      //  current_node->material = context.materials.front();
      //  current_node->material.update_id();
      //}
    }
    else
      current_node->material = material_iter->second;

    if (has_tokens())
    {
      log::warning() << "Ignoring additional arguments of NamedMaterial token.";
    }
  }
  else if (token == "Shape")
  {
    auto* current_node = context.current_nodes.top().get();
    // auto& current_scope = context.scope.top();
    auto shape_type = get_token();
    if (shape_type == "trianglemesh" || shape_type == "curve" ||
        shape_type == "sphere")
    {
      /// ToDo: Add support for these shape types.
      return true;
    }
    else if (shape_type == "plymesh")
    {
      token = get_token();
      std::string type;
      std::string name;
      std::tie(type, name) = split_type(token);
      if (name == "filename")
      {
        if (type == "string")
        {
          auto current_pass = context.job.rule->pass;
          auto* mesh_file = context.compiler.alias(
            context.compiler.get_file(include_path / get_token()),
            current_pass);
          if (!mesh_file)
            return false;
          /// ToDo: Pass the exact path using rule options!
          current_node->mesh = repository.load<resource::mesh>(mesh_file->path);
          if (!current_node->mesh)
          {
            log::warning() << "Cannot find mesh \"" << mesh_file->generic_string
                           << "\".";
          }
        }
        else
        {
          log::warning() << R"(Ignoring attribute "filename" of unknown type ")"
                         << type << "\".";
          return true;
        }
      }
      else
      {
        log::warning() << "Skipping unknown shape attribute \"" << token
                       << "\".";
        return true;
      }
      /// ToDo: parse additional attributes
    }
    else
    {
      log::warning() << "Ignoring unknown shape type \"" << shape_type << "\".";
      return true;
    }
  }
  else if (token == "Translate")
  {
    std::array<float, 3> values{};
    for (auto& value : values)
      value = boost::lexical_cast<float>(get_token());
  }
  else if (token == "Rotate")
  {
    std::array<float, 4> values{};
    for (auto& value : values)
      value = boost::lexical_cast<float>(get_token());
  }
  else if (token == "Scale")
  {
    std::array<float, 3> values{};
    for (auto& value : values)
      value = boost::lexical_cast<float>(get_token());
  }
  else if (token == "Transform" || token == "ConcatTransform")
  {
    get_token();  // Ignore '['.
    std::array<float, 16> values{};
    for (auto& value : values)
      value = boost::lexical_cast<float>(get_token());
    get_token();  // Ignore ']'.
    auto matrix = math::make_matrix_from_column_major<4, 4>(values);
    /// ToDo: Find difference between Transform and ConcatTransform.
    if (!context.current_nodes.empty())
      context.current_nodes.top()->transform = matrix;
  }
  else if (token == "LookAt")
  {
    std::array<float, 9> values{};
    for (auto& value : values)
      value = boost::lexical_cast<float>(get_token());
    // auto matrix = math::make_matrix_from_column_major<3, 3>(values);
  }
  else if (token == "TransformBegin")
  {
    /// ToDo: Find difference between Transform and TransformBegin.
  }
  else if (token == "TransformEnd")
  {
  }
  else if (token == "ObjectInstance")
  {
    std::string instance_name = get_token();
    auto node_iter = context.named_nodes.find(instance_name);
    if (node_iter == context.named_nodes.end())
    {
      log::warning()
        << R"(Ignoring "ObjectInstance" token referencing unknown object ")"
        << instance_name << R"(".)";
      return false;
    }
    if (context.current_nodes.top()->transform ==
        math::make_identity_matrix<4, 4, float>())
    {
      log::warning()
        << "Instanced objects should have a transformation matrix.";
    }
    context.current_nodes.top()->children.push_back(node_iter->second);
  }
  else if (token == "Camera")
  {
    std::string name = get_token();
  }
  else if (token == "LightSource" || token == "AreaLightSource")
  {
    std::string name = get_token();
  }
  else if (token == "Sampler")
  {
    std::string name = get_token();
  }
  else if (token == "Film" || token == "Integrator")
  {
    /// ToDo: Add support for these tags.
  }
  else if (token == "Include")
  {
    if (!parse_file(context, base_path, include_path,
                    include_path / get_token()))
    {
      return false;
    }
  }
  else
  {
    log::debug() << "Skipping unsupported PBRT tag \"" << token << "\".";
  }
  return true;
}

bool action_scene_import_pbrt::parse_file(
  action_scene_import_pbrt::parser_context& context, const fs::path& base_path,
  const fs::path& include_path, const fs::path& file_path) const
{
  std::ifstream file;
  file.open((base_path / file_path).generic_string(), std::ios_base::in);
  if (!file.is_open())
  {
    /// ToDo: Throw exception.
    return false;
  }

  std::string content{std::istreambuf_iterator<char>(file),
                      std::istreambuf_iterator<char>()};
  std::size_t token_begin = 0;

  std::vector<std::string> token_group;
  for (;;)
  {
    // Skip whitespace.
    token_begin = content.find_first_not_of(" \n\r\t", token_begin);
    if (token_begin == std::string::npos)
      break;
    else if (content[token_begin] == '#')
    {
      // Found comment, so skip the remaining line and try again.
      token_begin = content.find_first_of("\n\r", token_begin);
      continue;
    }
    else if (content[token_begin] == '[' || content[token_begin] == ']')
    {
      token_group.emplace_back(content.c_str() + token_begin, 1);
      ++token_begin;
    }
    else if (content[token_begin] == '"')
    {
      // Found quoted token. Extract all text between quotes.
      ++token_begin;
      auto token_end = content.find_first_of('"', token_begin);
      if (token_end == std::string::npos)
      {
        BOOST_THROW_EXCEPTION(rc_error()
                              << core::context_info("Missing quotation mark."));
      }
      auto token =
        std::string{content.c_str() + token_begin, token_end - token_begin};
      BOOST_ASSERT(!token.empty());
      token_begin = token_end + 1;
      token_group.push_back(std::move(token));
    }
    else
    {
      // Found unquoted token. Extract text until next whitespace
      // character.
      std::string token;
      auto token_end = content.find_first_of("] \n\r\t", token_begin);
      if (token_end != std::string::npos)
      {
        token =
          std::string{content.c_str() + token_begin, token_end - token_begin};
      }
      else
      {
        token = std::string{content.c_str() + token_begin,
                            content.size() - token_begin};
      }
      BOOST_ASSERT(!token.empty());
      token_begin = token_end;
      if (std::isalpha(static_cast<unsigned char>(token[0])))
      {
        if (!token_group.empty())
        {
          if (!parse_token_group(context, token_group, base_path, include_path))
            return false;
          token_group.clear();
        }
      }
      token_group.push_back(std::move(token));
    }
  }
  if (!token_group.empty())
  {
    if (!parse_token_group(context, token_group, base_path, include_path))
      return false;
    token_group.clear();
  }

  if (file.fail() && !file.eof())
  {
    log::error() << "Unexpected file read error.";
    return false;
  }

  return true;
}
}
