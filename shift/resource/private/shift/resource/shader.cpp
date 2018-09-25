#include "shift/resource/shader.h"
#include <shift/serialization2/all.h>
#include <shift/crypto/sha256.h>

namespace shift::resource
{
serialization2::compact_input_archive<>& operator>>(
  serialization2::compact_input_archive<>& archive, stage_binding& binding)
{
  archive >> binding.usage >> binding.type >> binding.location >>
    binding.component;
  return archive;
}

serialization2::compact_output_archive<>& operator<<(
  serialization2::compact_output_archive<>& archive,
  const stage_binding& binding)
{
  archive << binding.usage << binding.type << binding.location
          << binding.component;
  return archive;
}

crypto::sha256& operator<<(crypto::sha256& context,
                           const stage_binding& binding)
{
  context << static_cast<std::uint32_t>(binding.usage)
          << static_cast<std::uint32_t>(binding.type) << binding.location
          << binding.component;
  return context;
}

void shader::load(resource_id id, boost::iostreams::filtering_istream& stream)
{
  _id = id;
  serialization2::compact_input_archive<> archive{stream};
  archive >> storage >> stage_inputs >> stage_outputs;
}

void shader::save(boost::iostreams::filtering_ostream& stream) const
{
  serialization2::compact_output_archive<> archive{stream};
  archive << storage << stage_inputs << stage_outputs;
}

void shader::hash(crypto::sha256& context) const
{
  context << "shader" << storage << stage_inputs << stage_outputs;
}
}
