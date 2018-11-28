#include "shift/render/vk/layer2/mesh.hpp"

#include <shift/log/log.hpp>

namespace shift::render::vk
{
mesh::~mesh() noexcept
{
}
}

namespace shift::render::vk::layer2
{
mesh::mesh(vk::layer1::device& /*device*/,
           mesh::vertex_attributes_t vertex_attributes,
           index_attribute_t index_attribute)
:  // _device(&device),
  _vertex_attributes(std::move(vertex_attributes)),
  _index_attribute(std::move(index_attribute))
{
  for (auto& vertex_attribute : _vertex_attributes)
  {
    if (vertex_attribute.buffer != nullptr)
    {
      if (vertex_attribute.buffer->add_parent(*this))
        ++_unloaded_children;
    }
  }
  if (_index_attribute.buffer != nullptr)
  {
    if (_index_attribute.buffer->add_parent(*this))
      ++_unloaded_children;
  }
  //  log::debug() << "Mesh #" << std::hex <<
  //  reinterpret_cast<std::size_t>(this)
  //               << " has " << std::dec << _unloaded_children
  //               << " unavailable dependencies.";
}

mesh::~mesh() noexcept
{
  if (_index_attribute.buffer != nullptr)
    _index_attribute.buffer->remove_parent(*this);
  for (auto& vertex_attribute : _vertex_attributes)
  {
    if (vertex_attribute.buffer != nullptr)
      vertex_attribute.buffer->remove_parent(*this);
  }
}

void mesh::signal(availability_state state)
{
  if (state == availability_state::unloaded)
  {
    if (++_unloaded_children == 1)
      availability(availability_state::unloaded);
  }
  else if (state == availability_state::loaded)
  {
    BOOST_ASSERT(_unloaded_children > 0);
    if (--_unloaded_children == 0)
      availability(availability_state::loaded);
  }
  else if (state == availability_state::updated)
  {
    if (_unloaded_children == 0)
      availability(availability_state::updated);
  }
  log::debug() << "Mesh #" << std::hex << reinterpret_cast<std::size_t>(this)
               << " now has " << std::dec << _unloaded_children
               << " unavailable dependencies left.";
}
}
