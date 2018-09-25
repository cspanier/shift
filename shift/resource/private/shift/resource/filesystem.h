#ifndef SHIFT_RESOURCE_FILESYSTEM_H
#define SHIFT_RESOURCE_FILESYSTEM_H

#include <cstdint>
#include <array>
#include <unordered_map>
#include <fstream>
#include <shared_mutex>
#include <shift/core/bit_field.h>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/filesystem/path.hpp>
#include "shift/resource/resource.h"
#include "shift/resource/mountable.h"

namespace shift::resource
{
///
class filesystem final : public mountable
{
public:
  ///
  filesystem(const boost::filesystem::path& path);

  ///
  ~filesystem() override;

  ///
  bool open(bool read_only = true) override;

  ///
  void close() override;

  ///
  resource_id lookup_id(const boost::filesystem::path& relative_path) override;

  ///
  bool load(resource_id id, resource_base& resource,
            resource_type type) override;

  ///
  bool save(const resource_base& resource, resource_type type, resource_id id,
            const boost::filesystem::path& relative_path) override;

  ///
  void erase(resource_id id) override;

private:
  struct index_entry
  {
    resource_id id;
    std::string generic_path;
  };

  struct by_id
  {
  };

  struct by_path
  {
  };

  using index_map = boost::multi_index_container<
    index_entry,
    boost::multi_index::indexed_by<
      boost::multi_index::ordered_non_unique<
        boost::multi_index::tag<by_id>,
        boost::multi_index::member<index_entry, resource_id, &index_entry::id>>,
      boost::multi_index::hashed_unique<
        boost::multi_index::tag<by_path>,
        boost::multi_index::member<index_entry, std::string,
                                   &index_entry::generic_path>>>>;

  ///
  bool save_impl(const resource_base& resource, resource_type type,
                 resource_id id, const boost::filesystem::path& relative_path);

  bool _read_only;
  std::shared_mutex _index_mutex;
  index_map _index;
};
}

#endif
