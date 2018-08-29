#ifndef SHIFT_CORE_HASH_TABLE_H
#define SHIFT_CORE_HASH_TABLE_H

#include <tuple>
#include <vector>
#include <shift/platform/assert.h>

namespace shift::core
{
/// A simple hash table.
template <typename Key, typename Value, class KeyCompare = std::equal_to<Key>,
          class Hash = std::hash<Key>>
class hash_table
{
public:
  using entry_t = std::pair<Key, Value>;

  /// Initializes the hash table with next_power_of_two(size) elements.
  hash_table(std::size_t size, const Hash& key_hash = Hash{},
             const KeyCompare& key_compare = KeyCompare{})
  : _key_hash(key_hash), _key_compare(key_compare)
  {
    _table.resize(math::next_power_of_two(size));
  }

  /// Lookup
  std::pair<bool, Value*> operator[](const Key& key) noexcept
  {
    std::size_t hash = _key_hash(key);
    std::size_t hash_mask = _table.size() - 1;
    std::size_t bucket = hash & hash_mask;

    for (std::size_t i = 0; i < hash_mask; ++i)
    {
      auto& item = _table[bucket];
      if (!item.first)
      {
        item.first = key;
        return std::make_pair(true, &item.second);
      }
      if (_key_compare(item.first, key))
        return std::make_pair(false, &item.second);
      // Handle hash collision.
      bucket = (bucket + i + 1) & hash_mask;
    }

    // Hash table is full.
    BOOST_ASSERT(false);
    return std::make_pair(false, nullptr);
  }

private:
  std::vector<entry_t> _table;
  Hash _key_hash;
  KeyCompare _key_compare;
};
}

#endif
