#include "shift/resource/material_descriptor.h"
#include <shift/serialization2/all.h>
#include <shift/math/serialization2.h>
#include <shift/crypto/sha256.h>

namespace shift::resource
{
void material_descriptor::load(resource_id /*id*/,
                               boost::iostreams::filtering_istream& /*stream*/)
{
  /// ToDo: implement...
}

void material_descriptor::save(
  boost::iostreams::filtering_ostream& /*stream*/) const
{
  /// ToDo: implement...
}

void material_descriptor::hash(crypto::sha256& context) const
{
  context << "material_descriptor";  // << storage << stage_inputs;
}
}
