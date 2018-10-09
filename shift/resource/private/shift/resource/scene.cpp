#include "shift/resource/scene.h"
#include <shift/serialization2/all.h>
#include <shift/crypto/sha256.h>
#include <shift/math/serialization2.h>

namespace shift::resource
{
///
serialization2::compact_input_archive<>& operator>>(
  serialization2::compact_input_archive<>& archive, scene_node& node)
{
  archive >> node.transform >> node.mesh >> node.material;
  return archive;
}

///
serialization2::compact_output_archive<>& operator<<(
  serialization2::compact_output_archive<>& archive, const scene_node& node)
{
  archive << node.transform << node.mesh << node.material;
  return archive;
}

crypto::sha256& operator<<(crypto::sha256& context, const scene_node& node)
{
  context << node.transform << node.mesh.id() << node.material.id();
  return context;
}

void scene::load(resource_id id, boost::iostreams::filtering_istream& stream)
{
  _id = id;
  serialization2::compact_input_archive<> archive{stream};
  std::uint32_t num_nodes;
  archive >> num_nodes;
  nodes.resize(num_nodes);
  for (auto& node : nodes)
  {
    node = std::make_unique<scene_node>();
    archive >> *node;
  }

  std::uint32_t index;
  for (auto& node : nodes)
  {
    std::uint32_t num_children;
    archive >> num_children;
    node->children.reserve(num_children);
    for (std::uint32_t i = 0; i < num_children; ++i)
    {
      archive >> index;
      auto* child = nodes[index].get();
      node->children.push_back(child);
    }
  }
  archive >> index;
  root = index < nodes.size() ? nodes[index].get() : nullptr;
}

void scene::save(boost::iostreams::filtering_ostream& stream) const
{
  serialization2::compact_output_archive<> archive{stream};
  auto num_nodes = static_cast<std::uint32_t>(nodes.size());
  archive << num_nodes;
  for (const auto& node : nodes)
    archive << *node;

  std::uint32_t index;
  for (const auto& node : nodes)
  {
    auto num_children = static_cast<std::uint32_t>(node->children.size());
    archive << num_children;
    for (const auto* child : node->children)
    {
      index = 0;
      for (const auto& inner_node : nodes)
      {
        if (inner_node.get() == child)
        {
          archive << index;
          break;
        }
        ++index;
      }
      BOOST_ASSERT(index < nodes.size());
    }
  }
  index = 0;
  for (const auto& node : nodes)
  {
    if (node.get() == root)
      break;
    ++index;
  }
  archive << index;
  BOOST_ASSERT(index < nodes.size());
}

void scene::hash(crypto::sha256& context) const
{
  for (const auto& node : nodes)
    context << *node;
  if (root)
    context << *root;
}

template <typename T, typename Container>
struct index_into
{
  T& value;
  Container& container;
};
}
