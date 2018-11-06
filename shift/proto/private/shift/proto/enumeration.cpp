#include "shift/proto/enumeration.hpp"
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
enumerator::~enumerator() = default;

void enumerator::generate_uids()
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
}

enumeration::~enumeration() = default;

void enumeration::generate_uids()
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
  for (auto& member : members)
    member.generate_uids();
}
}
