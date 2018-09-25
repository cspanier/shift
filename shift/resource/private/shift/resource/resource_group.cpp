#include "shift/resource/resource_group.h"
#include "shift/resource/resource_ptr.h"
#include "shift/resource/image.h"
#include "shift/resource/mesh.h"
#include "shift/resource/repository.h"
#include <shift/serialization2/all.h>
#include <shift/crypto/sha256.h>

namespace shift::resource
{
void resource_group::load(resource_id id,
                          boost::iostreams::filtering_istream& stream)
{
  _id = id;

  auto& repository = resource::repository::singleton_instance();
  std::uint32_t object_count;

  serialization2::compact_input_archive<> archive{stream};
  archive >> object_count;
  for (images.reserve(object_count); object_count > 0; --object_count)
  {
    resource_id image_id;
    auto new_image = std::make_shared<image>();
    archive >> image_id;
    new_image->load(image_id, stream);
    repository.add(new_image, image_id);
    images.emplace_back(std::move(new_image), image_id);
  }

  archive >> object_count;
  for (meshes.reserve(object_count); object_count > 0; --object_count)
  {
    resource_id mesh_id;
    auto new_mesh = std::make_shared<mesh>();
    archive >> mesh_id;
    new_mesh->load(mesh_id, stream);
    repository.add(new_mesh, mesh_id);
    meshes.emplace_back(std::move(new_mesh), mesh_id);
  }
}

void resource_group::save(boost::iostreams::filtering_ostream& stream) const
{
  serialization2::compact_output_archive<> archive{stream};
  archive << static_cast<std::uint32_t>(images.size());
  for (const auto& image : images)
  {
    archive << image.id();
    image.get_shared()->save(stream);
  }

  archive << static_cast<std::uint32_t>(meshes.size());
  for (const auto& mesh : meshes)
  {
    archive << mesh.id();
    mesh.get_shared()->save(stream);
  }
}

void resource_group::hash(crypto::sha256& context) const
{
  context << "resource_group";
  for (const auto& image : images)
    context << image.id();
  for (const auto& mesh : meshes)
    context << mesh.id();
}
}
