#ifndef SHIFT_PROTO_HASH_H
#define SHIFT_PROTO_HASH_H

//#include <functional>
#include <shift/crypto/sha256.h>
#include "shift/proto/types.h"

namespace shift::crypto
{
/// Hash overload for type ::shift::proto::type_reference.
sha256& operator<<(sha256& context,
                   const proto::type_reference& type_reference);

/// Hash overload for type ::shift::proto::alias.
sha256& operator<<(sha256& context, const ::shift::proto::alias& alias);

/// Hash overload for type ::shift::proto::enumerator.
sha256& operator<<(sha256& context, const ::shift::proto::enumerator& member);

/// Hash overload for type ::shift::proto::enumeration.
sha256& operator<<(sha256& context,
                   const ::shift::proto::enumeration& enumeration);

/// Hash overload for type ::shift::proto::field.
sha256& operator<<(sha256& context, const ::shift::proto::field& field);

/// Hash overload for type ::shift::proto::message.
sha256& operator<<(sha256& context, const ::shift::proto::message& message);

/// Hash overload for type ::shift::proto::interface.
sha256& operator<<(sha256& context, const ::shift::proto::interface& interface);

/// Hash overload for type ::shift::proto::service.
sha256& operator<<(sha256& context, const ::shift::proto::service& service);

/// Hash overload for type ::shift::proto::namescope.
sha256& operator<<(sha256& context, const ::shift::proto::namescope& namescope);
}

#endif
