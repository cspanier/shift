#include "shift/render/vk/layer2/object.hpp"
#include <boost/assert.hpp>

namespace shift::render::vk::layer2
{
parent_object::~parent_object() noexcept = default;

bool child_object::add_parent(parent_object& parent)
{
  /// ToDo: Check precondition in debug builds.
  auto insert_success = _parents.insert(&parent).second;
  return insert_success && (_availability == availability_state::unloaded);
}

bool child_object::remove_parent(parent_object& parent)
{
  /// ToDo: Check precondition in debug builds.
  auto erase_success = _parents.erase(&parent) > 0;
  return erase_success && (_availability == availability_state::unloaded);
}

availability_state child_object::availability() const noexcept
{
  return _availability;
}

void child_object::availability(availability_state new_state)
{
  auto parent_signal = new_state;
  if (_availability == availability_state::unloaded)
  {
    switch (new_state)
    {
    case availability_state::unloaded:
      return;

    case availability_state::loaded:
      _availability = new_state;
      break;

    case availability_state::updated:
      BOOST_ASSERT(_availability == availability_state::unloaded &&
                   new_state != availability_state::updated);
      return;
    }
  }
  else if (_availability == availability_state::loaded)
  {
    switch (new_state)
    {
    case availability_state::unloaded:
      _availability = new_state;
      break;

    case availability_state::loaded:
      parent_signal = availability_state::updated;
      break;

    case availability_state::updated:
      break;
    }
  }
  else
  {
    BOOST_ASSERT(_availability == availability_state::unloaded ||
                 _availability == availability_state::loaded);
    return;
  }

  for (auto* parent : _parents)
    parent->signal(parent_signal);
}
}
