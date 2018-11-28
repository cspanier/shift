#include "shift/resource_db/archive.hpp"
#include "shift/resource_db/repository.hpp"
#include <shift/serialization2/all.hpp>
#include <shift/core/substream_device.hpp>
#include <shift/core/exception.hpp>
#include <shift/core/string_util.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <array>

#include <iostream>

namespace shift::resource_db
{
static constexpr std::array<char, 4> archive_magic = {{'R', 'E', 'P', 'O'}};

serialization2::compact_input_archive<>& operator>>(
  serialization2::compact_input_archive<>& archive, archive_entry& entry)
{
  archive >> entry.id >> entry.type >> entry.flags >> entry.offset >>
    entry.size;
  return archive;
}

serialization2::compact_output_archive<>& operator<<(
  serialization2::compact_output_archive<>& archive, const archive_entry& entry)
{
  archive << entry.id << entry.type << entry.flags << entry.offset
          << entry.size;
  return archive;
}

serialization2::compact_input_archive<>& operator>>(
  serialization2::compact_input_archive<>& archive, named_resource& entry)
{
  std::uint64_t last_write_time;
  archive >> entry.id >> entry.name >> last_write_time;
  entry.last_write_time = static_cast<std::time_t>(last_write_time);
  return archive;
}

serialization2::compact_output_archive<>& operator<<(
  serialization2::compact_output_archive<>& archive,
  const named_resource& entry)
{
  auto last_write_time = static_cast<std::uint64_t>(entry.last_write_time);
  archive << entry.id << entry.name << last_write_time;
  return archive;
}

archive::archive(const boost::filesystem::path& path) : mountable(path)
{
}

archive::~archive()
{
  close();
}

bool archive::open(bool read_only)
{
  namespace fs = boost::filesystem;

  _read_only = read_only;

  if (!fs::exists(_path))
  {
    // Create an empty archive.
    if (_read_only)
    {
      BOOST_THROW_EXCEPTION(
        core::file_open_error()
        << core::context_info("Repository path references a non-existing file.")
        << core::path_name_info(_path.generic_string()));
    }

    _file.open(_path.generic_string(), std::ios_base::in | std::ios_base::out |
                                         std::ios_base::binary |
                                         std::ios_base::trunc);
    if (!_file.is_open())
    {
      BOOST_THROW_EXCEPTION(
        core::file_open_error()
        << core::context_info("Cannot create new archive file.")
        << core::path_name_info(_path.generic_string()));
    }
    _header.magic = archive_magic;
    _header.header_size = sizeof(_header);
    _header.data_size = 0;
    _header.dictionary_size = 0;
  }
  else if (!fs::is_regular_file(_path))
  {
    BOOST_THROW_EXCEPTION(
      core::path_error()
      << core::context_info(
           "Repository path does not reference a regular file.")
      << core::path_name_info(_path.generic_string()));
  }
  else
  {
    if (_read_only)
    {
      _file.open(_path.generic_string(),
                 std::ios_base::in | std::ios_base::binary);
      if (!_file.is_open())
      {
        BOOST_THROW_EXCEPTION(
          core::file_open_error()
          << core::context_info("Cannot open archive file for reading.")
          << core::path_name_info(_path.generic_string()));
      }
    }
    else
    {
      _file.open(_path.generic_string(), std::ios_base::in |
                                           std::ios_base::out |
                                           std::ios_base::binary);
      if (!_file.is_open())
      {
        BOOST_THROW_EXCEPTION(
          core::file_open_error()
          << core::context_info(
               "Cannot open archive file for reading and writing.")
          << core::path_name_info(_path.generic_string()));
      }
    }

    _file.seekg(0, std::ios_base::end);
    auto file_size = _file.tellg();
    _file.seekg(0, std::ios_base::beg);

    // Read file dictionary offset.
    if (file_size < static_cast<std::streamoff>(sizeof(_header)))
      return false;
    _file.read(reinterpret_cast<char*>(&_header), sizeof(_header));
    if (_header.magic != archive_magic ||
        _header.header_size != sizeof(_header))
    {
      // Referenced file is not a valid archive, so reset it.
      _header.magic = archive_magic;
      _header.header_size = sizeof(_header);
      _header.data_size = 0;
      _header.dictionary_size = 0;
    }

    if (_header.dictionary_size > 0)
    {
      boost::iostreams::filtering_istream stream;
      stream.push(boost::iostreams::zlib_decompressor());
      stream.push(core::substream_device<std::fstream>(
        _file, _header.header_size + _header.data_size,
        _header.dictionary_size));
      /// ToDo: serialization does not yet support boost multi_index
      /// containers. Thus we have to copy all data into a regular vector.
      std::vector<archive_entry> temp_dictionary;
      std::vector<named_resource> temp_names;
      serialization2::compact_input_archive<> archive{stream};
      archive >> temp_dictionary >> temp_names;
      for (auto& entry : temp_dictionary)
      {
        entry.source = this;
        _dictionary.insert(std::move(entry));
      }
      for (auto& name : temp_names)
        _names.insert(name);
    }
  }
  return true;
}

void archive::close()
{
  if (_file.is_open())
  {
    if (!_read_only)
    {
      std::vector<char> dictionary_buffer;
      // First, store object into a memory buffer.
      {
        boost::iostreams::filtering_ostream stream;
        stream.push(boost::iostreams::zlib_compressor());
        stream.push(boost::iostreams::back_inserter(dictionary_buffer));
        /// ToDo: serialization does not yet support boost multi_index
        /// containers. Thus we have to copy all data into a regular vector.
        std::vector<archive_entry> temp_dictionary;
        std::vector<named_resource> temp_names;
        for (const auto& entry : _dictionary)
          temp_dictionary.push_back(entry);
        for (const auto& name : _names)
          temp_names.push_back(name);
        serialization2::compact_output_archive<> archive{stream};
        archive << temp_dictionary << temp_names;
      }
      _header.dictionary_size = dictionary_buffer.size();

      _file.seekp(0, std::ios_base::beg);
      _file.write(reinterpret_cast<const char*>(&_header), sizeof(_header));
      _file.seekp(_header.header_size + _header.data_size, std::ios_base::beg);
      _file.write(dictionary_buffer.data(), dictionary_buffer.size());
    }
    _file.close();
  }
}

resource_id archive::lookup_id(const boost::filesystem::path& /*relative_path*/)
{
  return 0;
}

bool archive::load(resource_id id, resource_base& resource,
                   resource_type /*type*/)
{
  std::shared_lock lock(_mutex);
  auto entry_iter = _dictionary.find(id);
  if (entry_iter == _dictionary.end())
    return false;

  auto& entry = *entry_iter;
  BOOST_ASSERT(entry.type == resource.type());
  if (entry.type != resource.type())
    return false;

  boost::iostreams::filtering_istream stream;
  if (entry.flags & file_flag::zlib_compressed)
    stream.push(boost::iostreams::zlib_decompressor());
  stream.push(core::substream_device<std::fstream>(
    _file, _header.header_size + entry.offset, entry.size));
  resource.load(id, stream);
  return true;
}

bool archive::save(const resource_base& resource, resource_type type,
                   resource_id id,
                   const boost::filesystem::path& /*relative_path*/)
{
  namespace fs = boost::filesystem;

  if (_read_only)
  {
    /// ToDo: Throw exception.
    return false;
  }

  {
    std::shared_lock lock(_mutex);
    if (_dictionary.find(id) != _dictionary.end())
    {
      // Object already exists.
      return true;
    }
  }

  archive_entry entry;
  entry.type = type;
  entry.source = this;

  std::vector<char> buffer;
  // First, store object into a memory buffer.
  {
    boost::iostreams::filtering_ostream stream;
    stream.push(boost::iostreams::back_inserter(buffer));
    resource.save(stream);
  }
  //{
  //  // Compress the buffer and test whether this yields a smaller size.
  //  std::vector<char> compressed_buffer;
  //  {
  //    serialization::compact_output_archive<> compact_archive(1);
  //    compact_archive.push(boost::iostreams::zlib_compressor());
  //    compact_archive.push(
  //      boost::iostreams::back_inserter(compressed_buffer));
  //    compact_archive.write(buffer.data(), buffer.size());
  //  }
  //  if (compressed_buffer.size() < buffer.size())
  //  {
  //    buffer.swap(compressed_buffer);
  //    entry.flags |= file_flag::zlib_compressed;
  //  }
  //}

  /// ToDo: Find free storage, instead of simply appending new data to the
  /// end.

  {
    std::unique_lock lock(_mutex);
    entry.id = id;
    entry.offset = _header.data_size;
    entry.size = buffer.size();
    _file.seekp(_header.header_size + entry.offset, std::ios_base::beg);
    _file.write(buffer.data(), entry.size);
    _header.data_size += entry.size;
    _dictionary.insert(std::move(entry));
  }
  return true;
}

void archive::erase(resource_id /*id*/)
{
  /// ToDo: ...
}

// void archive::add_named_resource(const std::string& name, resource_id id,
//                                 std::time_t last_write_time)
//{
//  std::unique_lock lock(_mutex);
//  _names.insert(named_resource{id, name, last_write_time});
//}

boost::filesystem::path archive::to_path(resource_id id)
{
  std::stringstream path;
  path << std::hex << std::setw(2) << std::setfill('0')
       << (id >> ((sizeof(resource_id) - 1) * 8)) << "/" << std::hex
       << std::setw((sizeof(resource_id) - 1) * 2) << std::setfill('0')
       << (id << 8 >> 8);
  return path.str();
}
}
