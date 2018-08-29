#include "shift/proto/alias.h"
#include "shift/proto/hash.h"
#include <shift/core/core.h>
#include <shift/platform/assert.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/crc.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <algorithm>
#include <cstdint>
#include <string>

namespace shift::proto
{
alias::~alias() = default;

void alias::generate_uids()
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

const type_reference& alias::actual_type_reference() const
{
  if (auto* sub_alias = reference.as_alias())
    return sub_alias->actual_type_reference();
  else
    return reference;
}
}
