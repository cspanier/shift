#include "shift/resource/material_descriptor.hpp"
#include <shift/serialization2/all.hpp>
#include <shift/math/serialization2.hpp>
#include <shift/crypto/sha256.hpp>

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
