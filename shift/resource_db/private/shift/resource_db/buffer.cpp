#include "shift/resource_db/buffer.hpp"
#include <shift/serialization2/all.hpp>
#include <shift/crypto/sha256.hpp>

namespace shift::resource_db
{
void buffer::load(resource_id id, boost::iostreams::filtering_istream& stream)
{
  _id = id;
  serialization2::compact_input_archive<> archive{stream};
  archive >> storage;
}

void buffer::save(boost::iostreams::filtering_ostream& stream) const
{
  serialization2::compact_output_archive<> archive{stream};
  archive << storage;
}

void buffer::hash(crypto::sha256& context) const
{
  context << "buffer" << storage;
}
}
