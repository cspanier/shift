#include "shift/proto/nodeunpacker.h"
#include "shift/proto/namescope.h"
#include "shift/proto/service.h"
#include "shift/proto/interface.h"
#include "shift/proto/message.h"
#include "shift/proto/enumeration.h"
#include "shift/proto/alias.h"

namespace shift::proto
{
node_unpacker::node_unpacker(namescope& parent) : _parent(parent)
{
}

void node_unpacker::operator()(namescope& namescope) const
{
  namescope.parent = &_parent;
  namescope.depth = _parent.depth + 1;
  _parent.namescopes.push_back(&namescope);
  for (auto& child : namescope.dynamic_children)
    std::visit(node_unpacker{namescope}, *child);
}

void node_unpacker::operator()(alias& alias) const
{
  alias.parent = &_parent;
  alias.depth = _parent.depth + 1;
  _parent.aliases.push_back(&alias);
}

void node_unpacker::operator()(enumeration& enumeration) const
{
  enumeration.parent = &_parent;
  enumeration.depth = _parent.depth + 1;
  for (auto& member : enumeration.members)
  {
    member.parent = &_parent;
    member.depth = _parent.depth + 1;
  }
  _parent.enumerations.push_back(&enumeration);
}

void node_unpacker::operator()(message& message) const
{
  message.parent = &_parent;
  message.depth = _parent.depth + 1;
  for (auto& field : message.fields)
  {
    field.parent = &_parent;
    field.depth = _parent.depth + 1;
  }
  _parent.messages.push_back(&message);
}

void node_unpacker::operator()(interface& interface) const
{
  interface.parent = &_parent;
  interface.depth = _parent.depth + 1;
  interface.attribute("is_interface", "yes");
  _parent.interfaces.push_back(&interface);
  _parent.namescopes.push_back(&interface);
  for (auto& child : interface.dynamic_children)
    std::visit(node_unpacker{interface}, *child);
}

void node_unpacker::operator()(service& service) const
{
  service.parent = &_parent;
  service.depth = _parent.depth + 1;
  _parent.services.push_back(&service);
}
}
