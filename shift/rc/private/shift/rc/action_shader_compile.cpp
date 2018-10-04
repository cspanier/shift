#include "shift/rc/action_shader_compile.h"
#include "shift/rc/resource_compiler_impl.h"
#include <shift/resource/shader.h>
#include <shift/log/log.h>
#include <boost/filesystem.hpp>
#include <queue>
#include <memory>
#include <spirv_cross/spirv_cross.hpp>
#include <shaderc/shaderc.hpp>

namespace shift::rc
{
static resource::stage_usage translate_stage_usage(const std::string& name)
{
  if (name == "in_position")
    return resource::stage_usage::position;
  else if (name == "in_normal")
    return resource::stage_usage::normal;
  else if (name == "in_texcoord")
    return resource::stage_usage::texcoord;
  else
  {
    log::warning() << "Cannot detect usage of variable named \"" << name
                   << "\".";
  }
  return resource::stage_usage::unknown;
}

static resource::stage_type translate_stage_type(
  const spirv_cross::SPIRType& type, const std::string& name)
{
  switch (type.basetype)
  {
  case spirv_cross::SPIRType::Float:
    if (type.vecsize == 1 && type.columns == 1)
      return resource::stage_type::float32;
    else if (type.vecsize == 2 && type.columns == 1)
      return resource::stage_type::vec2;
    else if (type.vecsize == 2 && type.columns == 2)
      return resource::stage_type::mat2;
    else if (type.vecsize == 3 && type.columns == 1)
      return resource::stage_type::vec3;
    else if (type.vecsize == 3 && type.columns == 3)
      return resource::stage_type::mat3;
    else if (type.vecsize == 4 && type.columns == 1)
      return resource::stage_type::vec4;
    else if (type.vecsize == 4 && type.columns == 4)
      return resource::stage_type::mat4;
    break;

  default:
    break;
  }

  log::warning() << "Cannot detect type of variable named \"" << name << "\".";
  return resource::stage_type::none;
}

static std::string type_to_string(const spirv_cross::SPIRType& type)
{
  switch (type.basetype)
  {
  case spirv_cross::SPIRType::Float:
    switch (type.vecsize)
    {
    case 1:
      switch (type.columns)
      {
      case 1:
        return "float";
      }

    case 2:
      switch (type.columns)
      {
      case 1:
        return "vec2";
      case 2:
        return "mat2";
      }

    case 3:
      switch (type.columns)
      {
      case 1:
        return "vec3";
      case 3:
        return "mat3";
      }

    case 4:
      switch (type.columns)
      {
      case 1:
        return "vec4";
      case 4:
        return "mat4";
      }
    }
    break;

  default:
    break;
  }

  return "unknown";
}

static std::string read_shader_source(const std::string& filename)
{
  std::ifstream file;
  file.open(filename, std::ios_base::in | std::ios_base::binary);
  if (!file.is_open())
  {
    BOOST_THROW_EXCEPTION(
      core::file_open_error()
      << core::context_info("Cannot open shader source file for reading.")
      << core::path_name_info(filename));
  }

  file.seekg(0, std::ios_base::end);
  auto file_size = file.tellg();
  if (file_size <= 0)
    return {};

  std::string source;
  source.resize(static_cast<std::size_t>(file.tellg()));
  file.seekg(0, std::ios_base::beg);
  file.read(source.data(), static_cast<std::streamsize>(source.size()));
  return source;
}

static void write_shader_assembly(const std::string& filename,
                                  std::string_view assembly)
{
  std::ofstream file;
  file.open(filename,
            std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  if (!file.is_open())
  {
    BOOST_THROW_EXCEPTION(
      core::file_open_error()
      << core::context_info("Cannot open assembly source file for writing.")
      << core::path_name_info(filename));
  }

  file.write(assembly.data(), static_cast<std::streamsize>(assembly.size()));
}

static std::pair<std::string, std::vector<std::uint32_t>> compile_glsl_to_spirv(
  std::string_view source, shaderc_shader_kind shader_kind,
  const std::string& filename, shaderc_optimization_level optimization_level)
{
  shaderc::Compiler shader_compiler;
  shaderc::CompileOptions options;
  /// ToDo: Fill all options from job options.
  // options.AddMacroDefinition("MY_DEFINE", "1");
  options.SetOptimizationLevel(optimization_level);
  if (optimization_level ==
      shaderc_optimization_level::shaderc_optimization_level_zero)
  {
    options.SetGenerateDebugInfo();
  }
  options.SetAutoBindUniforms(false);
  options.SetAutoMapLocations(false);

  /// ToDo: Read entry point name from options.
  auto assembly = shader_compiler.CompileGlslToSpvAssembly(
    source.data(), source.size(), shader_kind, filename.c_str(), "main",
    options);
  auto module = shader_compiler.AssembleToSpv(
    assembly.cbegin(),
    static_cast<std::size_t>(std::distance(assembly.cbegin(), assembly.cend())),
    options);

  if (module.GetCompilationStatus() != shaderc_compilation_status_success)
  {
    /// ToDo: Eventually add source file name and action name.
    log::error() << module.GetErrorMessage();
    return {};
  }
  return {{assembly.cbegin(), assembly.cend()},
          {module.cbegin(), module.cend()}};
}

action_shader_compile::action_shader_compile()
: action_base(action_name, action_version)
{
}

bool action_shader_compile::process(resource_compiler_impl& compiler,
                                    job_description& job) const
{
  namespace fs = boost::filesystem;

  if (job.inputs.size() != 1)
  {
    log::error() << "The " << action_shader_compile::action_name
                 << " action can only process one input at a time.";
    return false;
  }
  if (!job.has_output("spirv"))
  {
    log::error() << "The " << action_shader_compile::action_name
                 << " action requires an output named \"spirv\".";
    return false;
  }
  const auto& input = job.inputs.front();
  if (!fs::exists(input->file->path) || !fs::is_regular_file(input->file->path))
  {
    log::error() << "Cannot find input file " << input->file->path << ".";
    return false;
  }
  auto extension = input->file->path.extension();
  shaderc_shader_kind shader_kind;
  if (extension == ".vert")
    shader_kind = shaderc_shader_kind::shaderc_vertex_shader;
  else if (extension == ".tesc")
    shader_kind = shaderc_shader_kind::shaderc_tess_control_shader;
  else if (extension == ".tese")
    shader_kind = shaderc_shader_kind::shaderc_tess_evaluation_shader;
  else if (extension == ".geom")
    shader_kind = shaderc_shader_kind::shaderc_geometry_shader;
  else if (extension == ".frag")
    shader_kind = shaderc_shader_kind::shaderc_fragment_shader;
  else if (extension == ".comp")
    shader_kind = shaderc_shader_kind::shaderc_compute_shader;
  else
  {
    log::error() << "Unknown input file extension.";
    return false;
  }

  auto source = read_shader_source(input->file->generic_string);
  auto optimized_spirv = compile_glsl_to_spirv(
    source, shader_kind, input->file->generic_string,
    shaderc_optimization_level::shaderc_optimization_level_performance);
  if (optimized_spirv.second.empty())
    return false;

  auto shader = std::make_shared<resource::shader>();
  {
    auto unoptimized_spirv = compile_glsl_to_spirv(
      source, shader_kind, input->file->generic_string,
      shaderc_optimization_level::shaderc_optimization_level_zero);
    if (unoptimized_spirv.second.empty())
      return false;

    // Reflect SPIR-V byte code and extract shader module interface.
    spirv_cross::Compiler spirv_compiler(unoptimized_spirv.second);
    auto resources = spirv_compiler.get_shader_resources();
    log::info info;
    info << input->file->path.generic_string();
    info << "\n  inputs:";
    for (const auto& stage_input : resources.stage_inputs)
    {
      resource::stage_binding binding{};
      binding.usage = translate_stage_usage(stage_input.name);
      binding.type = translate_stage_type(
        spirv_compiler.get_type(stage_input.type_id), stage_input.name);
      binding.location =
        spirv_compiler.get_decoration(stage_input.id, spv::DecorationLocation);
      binding.component =
        spirv_compiler.get_decoration(stage_input.id, spv::DecorationComponent);
      shader->stage_inputs.push_back(binding);

      info << "\n    "
           << type_to_string(spirv_compiler.get_type(stage_input.type_id))
           << " " << stage_input.name;
      info << " location=" << binding.location;
      info << " component=" << binding.component;
    }
    info << "\n  outputs:";
    for (const auto& stage_output : resources.stage_outputs)
    {
      resource::stage_binding binding{};
      binding.usage = translate_stage_usage(stage_output.name);
      binding.type = translate_stage_type(
        spirv_compiler.get_type(stage_output.type_id), stage_output.name);
      binding.location =
        spirv_compiler.get_decoration(stage_output.id, spv::DecorationLocation);
      binding.component = spirv_compiler.get_decoration(
        stage_output.id, spv::DecorationComponent);
      shader->stage_outputs.push_back(binding);

      info << "\n    "
           << type_to_string(spirv_compiler.get_type(stage_output.type_id))
           << " " << stage_output.name;
      info << " location=" << binding.location;
      info << " component=" << binding.component;
    }
    info << "\n  uniform buffers:";
    for (const auto& uniform_buffer : resources.uniform_buffers)
    {
      info << "\n    name=" << uniform_buffer.name;
      info << " set="
           << spirv_compiler.get_decoration(uniform_buffer.id,
                                            spv::DecorationDescriptorSet);
      info << " binding="
           << spirv_compiler.get_decoration(uniform_buffer.id,
                                            spv::DecorationBinding);
    }
    info << "\n  images:";
    for (const auto& separate_image : resources.separate_images)
    {
      info << "\n    name=" << separate_image.name;
      info << " set="
           << spirv_compiler.get_decoration(separate_image.id,
                                            spv::DecorationDescriptorSet);
      info << " binding="
           << spirv_compiler.get_decoration(separate_image.id,
                                            spv::DecorationBinding);
    }
    info << "\n  samplers:";
    for (const auto& sampler : resources.separate_samplers)
    {
      info << "\n    name=" << sampler.name;
      info << " set="
           << spirv_compiler.get_decoration(sampler.id,
                                            spv::DecorationDescriptorSet);
      info << " binding="
           << spirv_compiler.get_decoration(sampler.id, spv::DecorationBinding);
    }
    info << "\n  sampled images:";
    for (const auto& sampled_image : resources.sampled_images)
    {
      info << "\n    name=" << sampled_image.name;
      info << " set="
           << spirv_compiler.get_decoration(sampled_image.id,
                                            spv::DecorationDescriptorSet);
      info << " binding="
           << spirv_compiler.get_decoration(sampled_image.id,
                                            spv::DecorationBinding);
    }
  }

  // Save assembly.
  if (job.has_output("assembly"))
  {
    auto assembly_filename = job.output("assembly", {});
    fs::create_directories(assembly_filename.parent_path());
    write_shader_assembly(assembly_filename.generic_string(),
                          optimized_spirv.first);
    compiler.push(assembly_filename, job);
  }

  // Save SPIR-V byte code.
  shader->storage = std::move(optimized_spirv.second);
  compiler.save(*shader, job.output("spirv", {}), job);
  return true;
}
}
