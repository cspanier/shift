#include "shift/proto/interface.hpp"
#include "shift/proto/service.hpp"
#include "shift/proto/message.hpp"
#include "shift/proto/enumeration.hpp"
#include "shift/proto/alias.hpp"
#include "shift/proto/hash.hpp"
#include <shift/core/core.hpp>
#include <shift/platform/assert.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/crc.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <algorithm>
#include <cstdint>
#include <string>

namespace shift::proto
{
interface::~interface() = default;

type* interface::find_type(const namescope_path& namescope_path,
                           std::size_t current_depth)
{
  if (namescope_path.size() <= current_depth)
    return nullptr;
  else if (namescope_path.size() == current_depth + 1)
  {
    for (auto& child : dynamic_children)
    {
      if (auto* message = std::get_if<proto::message>(child.get()))
      {
        if (message->name == namescope_path[current_depth])
          return child.get();
      }
    }
  }
  else
  {
    for (auto& child : dynamic_children)
    {
      if (auto* message = std::get_if<proto::message>(child.get()))
      {
        if (message->name == namescope_path[current_depth])
          return message->find_type(namescope_path, current_depth + 1);
      }
    }
  }

  if (current_depth == 0 && (parent != nullptr))
    return parent->find_type(namescope_path);
  else
    return nullptr;
}

void interface::generate_uids()
{
  if (!has_attribute("uid"))
  {
    crypto::sha256::digest_t hash{};
    crypto::sha256 context(hash);
    (context << *this).finalize();
    boost::crc_32_type crc;
    for (auto c : hash)
      crc(c);
    attribute("uid", crc(), attribute_assignment::keep);
  }
  for (auto& message : messages)
    message->generate_uids();
}
}
