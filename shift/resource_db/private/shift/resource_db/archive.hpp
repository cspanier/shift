#ifndef SHIFT_RESOURCE_DB_ARCHIVE_HPP
#define SHIFT_RESOURCE_DB_ARCHIVE_HPP

#include <cstdint>
#include <array>
#include <unordered_map>
#include <fstream>
#include <shared_mutex>
#include <shift/core/bit_field.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/filesystem/path.hpp>
#include "shift/resource_db/resource.hpp"
#include "shift/resource_db/mountable.hpp"

namespace shift::resource_db
{
enum class file_flag : std::uint32_t
{
  zlib_compressed = 0b0001,
  bzip2_compressed = 0b0010
};

using file_flags = core::bit_field<file_flag>;

// Data header used for archive files.
struct archive_header
{
  std::array<char, 4> magic;
  std::uint32_t header_size;
  std::uint64_t data_size;
  std::uint64_t dictionary_size;
};

class archive;

struct archive_entry
{
  ///
  friend serialization2::compact_input_archive<>& operator>>(
    serialization2::compact_input_archive<>& archive, archive_entry& entry);

  ///
  friend serialization2::compact_output_archive<>& operator<<(
    serialization2::compact_output_archive<>& archive,
    const archive_entry& entry);

  resource_id id = 0;
  resource_type type = resource_type::undefined;
  file_flags flags = file_flags{0};
  std::uint64_t offset = 0;
  std::uint64_t size = 0;
  mutable archive* source = nullptr;
  mutable std::shared_ptr<resource_base> instance;
};

struct named_resource
{
  ///
  friend serialization2::compact_input_archive<>& operator>>(
    serialization2::compact_input_archive<>& archive, named_resource& entry);

  ///
  friend serialization2::compact_output_archive<>& operator<<(
    serialization2::compact_output_archive<>& archive,
    const named_resource& entry);

  resource_id id;
  std::string name;
  std::time_t last_write_time;
};

struct id_tag
{
};

struct type_tag
{
};

struct name_tag
{
};

struct last_write_time_tag
{
};

using resource_set = boost::multi_index_container<
  archive_entry,
  boost::multi_index::indexed_by<
    boost::multi_index::ordered_unique<boost::multi_index::tag<id_tag>,
                                       BOOST_MULTI_INDEX_MEMBER(
                                         archive_entry, resource_id, id)>,
    boost::multi_index::ordered_non_unique<
      boost::multi_index::tag<type_tag>,
      BOOST_MULTI_INDEX_MEMBER(archive_entry, resource_type, type)>>>;

using named_resource_set = boost::multi_index_container<
  named_resource,
  boost::multi_index::indexed_by<
    boost::multi_index::ordered_unique<boost::multi_index::tag<id_tag>,
                                       BOOST_MULTI_INDEX_MEMBER(
                                         named_resource, resource_id, id)>,
    boost::multi_index::ordered_unique<boost::multi_index::tag<name_tag>,
                                       BOOST_MULTI_INDEX_MEMBER(
                                         named_resource, std::string, name)>>>;
/// An archive is the storage class for the repository key-value database.
class archive final : public mountable
{
public:
  ///
  archive(const boost::filesystem::path& path);

  ///
  ~archive() override;

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

  ///
  const resource_set& dictionary() const
  {
    return _dictionary;
  }

  ///
  const named_resource_set& names() const
  {
    return _names;
  }

private:
  ///
  static boost::filesystem::path to_path(resource_id id);

  bool _read_only = true;

  std::fstream _file;
  archive_header _header;
  std::shared_timed_mutex _mutex;
  resource_set _dictionary;
  named_resource_set _names;
};
}

#endif
