#include "shift/resource/image.h"
#include <shift/serialization2/all.h>
#include <shift/crypto/sha256.h>

namespace shift::resource
{
///
serialization2::compact_input_archive<>& operator>>(
  serialization2::compact_input_archive<>& archive, mipmap_info& mipmap)
{
  archive >> mipmap.buffer >> mipmap.offset >> mipmap.width >> mipmap.height >>
    mipmap.depth;
  mipmap.width = std::max(mipmap.width, 1u);
  mipmap.height = std::max(mipmap.height, 1u);
  mipmap.depth = std::max(mipmap.depth, 1u);
  return archive;
}

///
serialization2::compact_output_archive<>& operator<<(
  serialization2::compact_output_archive<>& archive, const mipmap_info& mipmap)
{
  archive << mipmap.buffer << mipmap.offset << mipmap.width << mipmap.height
          << mipmap.depth;
  return archive;
}

void image::load(resource_id id, boost::iostreams::filtering_istream& stream)
{
  _id = id;
  serialization2::compact_input_archive<> archive{stream};
  archive >> format >> array_element_count >> face_count >> mipmaps;
}

void image::save(boost::iostreams::filtering_ostream& stream) const
{
  serialization2::compact_output_archive<> archive{stream};
  archive << format << array_element_count << face_count << mipmaps;
}

void image::hash(crypto::sha256& context) const
{
  context << "image" << format << array_element_count << face_count;
  for (const auto& mipmap : mipmaps)
  {
    context << mipmap.buffer.id() << mipmap.offset << mipmap.width
            << mipmap.height << mipmap.depth;
  }
}
}
