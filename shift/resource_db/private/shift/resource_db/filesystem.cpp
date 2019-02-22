#include "shift/resource_db/filesystem.hpp"
#include "shift/resource_db/repository.hpp"
#include <shift/parser/json/json.hpp>
#include <shift/serialization2/all.hpp>
#include <shift/core/substream_device.hpp>
#include <shift/core/exception.hpp>
#include <shift/core/string_util.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <filesystem>
#include <fstream>
#include <array>

namespace shift::resource_db
{
namespace fs = std::filesystem;

static const std::string index_filename = ".index.json";

filesystem::filesystem(const std::filesystem::path& path) : mountable(path)
{
}

filesystem::~filesystem()
{
  close();
}

bool filesystem::open(bool read_only)
{
  using namespace shift::parser;

  _read_only = read_only;

  if (!fs::exists(_path))
  {
    if (!fs::create_directories(_path))
    {
      BOOST_THROW_EXCEPTION(
        core::io_error() << core::context_info("Failed creating directories.")
                         << core::path_name_info(_path.generic_string()));
    }
  }
  else if (!fs::is_directory(_path))
  {
    BOOST_THROW_EXCEPTION(
      core::io_error() << core::context_info("Path must point to a directory.")
                       << core::path_name_info(_path.generic_string()));
  }

  std::ifstream index_file;
  index_file.open((_path / index_filename).generic_string(),
                  std::ios::in | std::ios_base::binary);
  // Simply ignore any errors from this point on.
  if (!index_file.is_open())
    return true;

  json::value root;
  try
  {
    index_file.unsetf(std::ios_base::skipws);
    index_file >> root;
    index_file.close();
  }
  catch (...)
  {
    // Ignore any errors during index file reading and parsing.
    return true;
  }

  if (json::get_if<json::object>(&root) == nullptr)
    return true;
  json::object& root_object = json::get<json::object>(root);

  if (!json::has(root_object, "index"))
    return true;
  const auto& index_value = root_object.at("index");
  const auto* index_object = json::get_if<json::object>(&index_value);
  if (index_object == nullptr)
    return true;

  for (const auto& key_object : *index_object)
  {
    const auto* index_array = json::get_if<json::array>(&key_object.second);
    if (index_array == nullptr)
      continue;
    for (const auto& value_object : *index_array)
    {
      const auto* relative_path = json::get_if<std::string>(&value_object);

      if ((relative_path != nullptr) && fs::exists(_path / *relative_path))
      {
        _index.insert(
          {static_cast<resource_id>(std::stoull(key_object.first, nullptr, 16)),
           *relative_path});
      }
    }
  }

  return true;
}

void filesystem::close()
{
  using namespace shift::parser;

  std::ofstream file(
    (_path / index_filename).generic_string(),
    std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  if (!file.is_open())
    return;

  const auto& id_index = _index.get<by_id>();
  json::object root;
  auto& index_object = json::get<json::object>(root["index"] = json::object{});

  // auto next = id_index.begin();
  for (auto first = id_index.begin(), last = id_index.upper_bound(first->id),
            end = id_index.end();
       first != end; first = last, last = id_index.upper_bound(first->id))
  {
    std::stringstream id_value;
    id_value << std::hex << std::setw(16) << std::setfill('0') << first->id;
    auto& entry_array =
      json::get<json::array>(index_object[id_value.str()] = json::array{});

    for (auto iter = first; iter != last; ++iter)
      entry_array.emplace_back(iter->generic_path);
  }

  file << core::indent_character(' ') << core::indent_width(2);
  file << root;
  file.close();
}

resource_id filesystem::lookup_id(const std::filesystem::path& relative_path)
{
  std::shared_lock read_lock(_index_mutex);
  const auto& path_index = _index.get<by_path>();
  auto index_iter = path_index.find(relative_path.generic_string());
  if (index_iter != path_index.end())
    return index_iter->id;
  return 0;
}

bool filesystem::load(resource_id id, resource_base& resource,
                      resource_type /*type*/)
{
  fs::path relative_path;
  {
    std::shared_lock read_lock(_index_mutex);
    const auto& id_index = _index.get<by_id>();
    auto index_iter = id_index.find(id);
    if (index_iter == id_index.end())
      return false;
    relative_path = index_iter->generic_path;
  }

  auto filename = _path / relative_path;
  if (!fs::exists(filename))
    return false;

  boost::iostreams::file_source file_source{
    filename.generic_string(), std::ios_base::in | std::ios_base::binary};
  if (!file_source.is_open())
  {
    BOOST_THROW_EXCEPTION(
      shift::core::file_open_error()
      << shift::core::file_name_info(filename.generic_string()));
  }

  boost::iostreams::filtering_istream stream;
  if (filename.extension().generic_string() == ".deflate")
    stream.push(boost::iostreams::zlib_decompressor());
  if (filename.extension().generic_string() == ".bz2")
    stream.push(boost::iostreams::bzip2_decompressor());
  stream.push(std::move(file_source));
  resource.load(id, stream);
  return true;
}

bool filesystem::save(const resource_base& resource, resource_type type,
                      resource_id id,
                      const std::filesystem::path& relative_path)
{
  if (_read_only)
  {
    /// ToDo: Throw exception.
    return false;
  }

  if (save_impl(resource, type, id, relative_path))
  {
    std::unique_lock write_lock(_index_mutex);
    return _index.insert({id, relative_path.generic_string()}).second;
  }
  else
    return false;
}

void filesystem::erase(resource_id id)
{
  {
    std::shared_lock read_lock(_index_mutex);
    const auto& id_index = _index.get<by_id>();
    auto index_iter = id_index.find(id);
    if (index_iter != id_index.end())
    {
      auto absolute_path = _path / index_iter->generic_path;
      if (fs::exists(absolute_path))
      {
        std::error_code error_code;
        fs::remove(absolute_path, error_code);
      }
    }
  }
}

bool filesystem::save_impl(const resource_base& resource,
                           resource_type /*type*/, resource_id id,
                           const std::filesystem::path& relative_path)
{
  auto absolute_path = _path / relative_path;
  fs::create_directories(absolute_path.parent_path());

  // Check whether a different file with the same id already exists. If so we
  // create a hard link to avoid duplicate data.
  {
    std::shared_lock read_lock(_index_mutex);
    const auto& id_index = _index.get<by_id>();
    auto index_iter = id_index.find(id);
    if (index_iter != id_index.end())
    {
      auto other_path = _path / index_iter->generic_path;
      if (absolute_path == other_path)
      {
        if (fs::exists(absolute_path))
          return true;
      }
      else if (fs::exists(other_path))
      {
        std::error_code error_code;
        fs::create_hard_link(other_path, absolute_path, error_code);
        /// ToDo: What to do if the file already exists? This may happen if only
        /// the rc db is deleted but output files are still there.
        // return !error_code;
        return true;
      }
    }
  }

  boost::iostreams::file_sink file_sink{
    absolute_path.generic_string(),
    std::ios_base::out | std::ios_base::trunc | std::ios_base::binary};
  if (!file_sink.is_open())
  {
    /// ToDo: Throw exception.
    return false;
  }

  std::vector<char> buffer;
  // First, store object into a memory buffer.
  {
    boost::iostreams::filtering_ostream stream;
    stream.push(boost::iostreams::back_inserter(buffer));
    resource.save(stream);
  }
  //{
  //  // compress the buffer and test whether this yields a smaller size.
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
  //    absolute_path += ".deflate";
  //  }
  //}

  file_sink.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
  return true;
}
}
