#include "shift/rc/action_image_export_tiff.hpp"
#include "shift/rc/resource_compiler_impl.hpp"
#include "shift/rc/image/tiff_io.hpp"
#include <shift/resource_db/image.hpp>
#include <shift/log/log.hpp>
#include <shift/math/utility.hpp>
#include <shift/math/vector.hpp>
#include <shift/core/mpl.hpp>
#include <shift/core/string_util.hpp>
#include <filesystem>
#include <gsl/gsl>
#include <compressonator/Compressonator.h>
#include <tiffio.h>

namespace shift::rc
{
namespace fs = std::filesystem;

action_image_export_tiff::action_image_export_tiff()
: action_base(action_name, action_version)
{
}

bool action_image_export_tiff::process(resource_compiler_impl& compiler,
                                       job_description& job) const
{
  using namespace std::string_literals;

  static tiff_io io;

  if (job.inputs.size() != 1)
  {
    log::error() << "The " << action_image_export_tiff::action_name
                 << " action can only process one input at a time.";
    return false;
  }
  const auto& input = *job.inputs.begin()->second;

  if (!fs::exists(input.file->path) || !fs::is_regular_file(input.file->path))
  {
    log::error() << "Cannot find input file " << input.file->path << ".";
    return false;
  }

  BOOST_ASSERT(job.rule);

  auto target_format =
    parser::json::has(job.rule->options, "target-format")
      ? parser::json::get<std::string>(job.rule->options.at("target-format"))
      : "auto"s;

  auto image = std::make_shared<resource_db::image>();

  input.file->alias =
    compiler.save(*image, job.output_file_path("header", {}), job);
  return false;
}
}
