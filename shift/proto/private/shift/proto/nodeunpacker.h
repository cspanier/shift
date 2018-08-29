#ifndef SHIFT_PROTO_NODEUNPACKER_H
#define SHIFT_PROTO_NODEUNPACKER_H

#include "shift/proto/types.h"

namespace shift::proto
{
/// Unpacks the vector of variant types into vectors of specific types to ease
/// usage.
struct node_unpacker
{
  using return_type = void;

  /// Constructor.
  node_unpacker(namescope& parent);

  ///
  void operator()(namescope& namescope) const;

  ///
  void operator()(alias& alias) const;

  ///
  void operator()(enumeration& enumeration) const;

  ///
  void operator()(message& message) const;

  ///
  void operator()(interface& interface) const;

  ///
  void operator()(service& service) const;

private:
  namescope& _parent;
};
}

#endif
