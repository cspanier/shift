#include "shift/rc/action_group_resources.h"
#include "shift/rc/resource_compiler_impl.h"
#include <shift/resource/resource_group.h>
#include <shift/resource/image.h>
#include <shift/resource/mesh.h>
#include <shift/log/log.h>
#include <shift/core/exception.h>
#include <boost/filesystem.hpp>
#include <queue>
#include <memory>

namespace shift::rc
{
action_group_resources::action_group_resources()
: action_base(action_name, action_version)
{
}

bool action_group_resources::process(resource_compiler_impl& compiler,
                                     job_description& job) const
{
  namespace fs = boost::filesystem;

  if (job.inputs.empty())
    return false;

  auto& repository = resource::repository::singleton_instance();
  auto group = std::make_shared<resource::resource_group>();
  for (const auto& [input_slot_index, input] : job.inputs)
  {
    if (input->slot->first == "images")
    {
      auto image = repository.load<resource::image>(input->file->path);
      if (!image)
      {
        log::error() << "Cannot add image " << input->file->path
                     << " to group.";
        continue;
      }
      group->images.push_back(image);
    }
    else if (input->slot->first == "meshes")
    {
      auto mesh = repository.load<resource::mesh>(input->file->path);
      if (!mesh)
      {
        log::error() << "Cannot add mesh " << input->file->path << " to group.";
        continue;
      }
      group->meshes.push_back(mesh);
    }
    else
    {
      log::error() << "Unknown pattern name \"" << input->slot->first
                   << "\" in rule \"" << job.rule->id << "\".";
    }
  }
  // Store resource into repository.
  compiler.save(*group, job.output("group", {}), job);

  return true;
}
}
