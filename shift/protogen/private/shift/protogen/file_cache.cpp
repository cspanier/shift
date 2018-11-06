#include "shift/protogen/file_cache.hpp"
#include <shift/parser/json/json.hpp>
#include <shift/log/log.hpp>
#include <shift/core/stream_util.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <fstream>
#include <string_view>

namespace shift::proto
{
bool convert(crypto::sha256::digest_t& output, std::string_view input)
{
  if (input.size() != output.size() * 2)
    return false;
  for (std::size_t index = 0; index < output.size(); ++index)
  {
    std::uint8_t o = 0;
    auto i1 = input[2 * index];
    auto i0 = input[2 * index + 1];

    if (i1 >= '0' && i1 <= '9')
      o = (i1 - '0') * 16;
    else if (i1 >= 'a' && i1 <= 'f')
      o = (i1 - 'a' + 10) * 16;
    else if (i1 >= 'A' && i1 <= 'F')
      o = (i1 - 'A' + 10) * 16;
    else
      return false;

    if (i0 >= '0' && i0 <= '9')
      o += i0 - '0';
    else if (i0 >= 'a' && i0 <= 'f')
      o += i0 - 'a' + 10;
    else if (i0 >= 'A' && i0 <= 'F')
      o += i0 - 'A' + 10;
    else
      return false;

    output[index] = o;
  }
  return true;
}

bool convert(std::string& output, const crypto::sha256::digest_t& input)
{
  output.resize(input.size() * 2);
  for (std::size_t index = 0; index < input.size(); ++index)
  {
    auto i1 = input[index] / 16;
    auto i0 = input[index] % 16;
    output[index * 2] = (i1 < 10) ? i1 + '0' : (i1 - 10) + 'A';
    output[index * 2 + 1] = (i0 < 10) ? i0 + '0' : (i0 - 10) + 'A';
  }
  return true;
}

void file_cache::base_path(boost::filesystem::path base_path)
{
  _base_path = std::move(base_path);
}

bool file_cache::read_cache(const boost::filesystem::path& cache_filename)
{
  std::ifstream cache_file;
  cache_file.open(cache_filename.generic_string(),
                  std::ios::in | std::ios_base::binary);
  if (!cache_file.is_open())
  {
    return false;
  }
  cache_file.unsetf(std::ios_base::skipws);

  try
  {
    using namespace shift::parser;
    json::value root;
    cache_file >> root;
    auto& root_object = json::get<json::object>(root);

    if (!convert(_combined_input_hash,
                 json::get<std::string>(root_object, "input-hash")))
    {
      return false;
    }
    for (const auto& output : json::get<json::array>(root_object, "outputs"))
    {
      auto output_object = json::get<json::object>(output);
      output_cache_entry entry;
      if (!convert(entry.pre_filter_hash,
                   json::get<std::string>(output_object, "pre-filter-hash")))
      {
        return false;
      }
      if (json::has(output_object, "post-filter-hash"))
      {
        entry.post_filter_hash = std::make_shared<crypto::sha256::digest_t>();
        if (!convert(*entry.post_filter_hash,
                     json::get<std::string>(output_object, "post-filter-hash")))
        {
          return false;
        }
      }
      _output_cache.insert(std::make_pair(
        (_base_path / json::get<std::string>(output_object, "filename"))
          .generic_string(),
        entry));
    }
  }
  catch (...)
  {
    return false;
  }
  return true;
}

bool file_cache::write_cache(
  const boost::filesystem::path& cache_filename,
  const crypto::sha256::digest_t& combined_input_hash)
{
  std::ofstream cache_file(
    cache_filename.generic_string(),
    std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
  if (!cache_file.is_open())
    return false;

  try
  {
    using namespace shift::parser;
    json::object root;

    std::string input_hash;
    if (!convert(input_hash, combined_input_hash))
      return false;
    root["input-hash"] = input_hash;

    json::array output_array;
    for (const auto& entry : _output_cache)
    {
      // Cut _base_path off each filename.
      boost::filesystem::path full_filename = entry.first;
      boost::filesystem::path truncated_filename;
      auto folder1 = _base_path.begin();
      auto folder2 = full_filename.begin();
      while (folder1 != _base_path.end() && folder2 != full_filename.end() &&
             *folder1 == *folder2)
      {
        ++folder1;
        ++folder2;
      }
      for (; folder2 != full_filename.end(); ++folder2)
        truncated_filename /= *folder2;
      json::object output_entry;
      output_entry["filename"] = truncated_filename.generic_string();

      std::string pre_filter_hash;
      std::string post_filter_hash;
      if (!convert(pre_filter_hash, entry.second.pre_filter_hash))
        return false;
      output_entry["pre-filter-hash"] = std::move(pre_filter_hash);

      if (entry.second.post_filter_hash)
      {
        if (!convert(post_filter_hash, *entry.second.post_filter_hash))
          return false;
        output_entry["post-filter-hash"] = std::move(post_filter_hash);
      }
      output_array.emplace_back(std::move(output_entry));
    }
    root["outputs"] = std::move(output_array);
    cache_file << core::indent_width(2) << root;
  }
  catch (...)
  {
    return false;
  }
  return true;
}

bool file_cache::check_input_hash(
  const crypto::sha256::digest_t& combined_input_hash)
{
  return std::memcmp(_combined_input_hash.data(), combined_input_hash.data(),
                     combined_input_hash.size()) == 0;
}

bool file_cache::check_output_hashes()
{
  namespace fs = boost::filesystem;
  for (const auto& output_file : _output_cache)
  {
    const auto& filename = output_file.first;
    if (!fs::exists(filename) || !fs::is_regular_file(filename))
      return false;

    std::string content;
    std::ifstream stream(filename, std::ios_base::in | std::ios_base::binary);
    if (!stream.is_open())
      return false;
    stream.unsetf(std::ios_base::skipws);
    std::copy(std::istream_iterator<char>(stream),
              std::istream_iterator<char>(), std::back_inserter(content));
    stream.close();

    // Normalize file content into Unix format.
    boost::replace_all(content, "\r\n", "\n");

    crypto::sha256::digest_t hash{};
    crypto::sha256 sha256(hash);
    sha256 << content;
    sha256.finalize();

    const auto* stored_hash = output_file.second.post_filter_hash
                                ? output_file.second.post_filter_hash.get()
                                : &output_file.second.pre_filter_hash;
    if (std::memcmp(stored_hash->data(), hash.data(), hash.size()) != 0)
      return false;
  }
  return true;
}

void file_cache::unlink_unchecked_files()
{
  using namespace boost::filesystem;

  auto i = _output_cache.begin();
  while (i != _output_cache.end())
  {
    if (!i->second.checked)
    {
      log::info() << "Removing obsolete file " << i->first;
      boost::system::error_code error;
      if (exists(i->first))
        remove(i->first, error);
      _output_cache.erase(i++);
    }
    else
      ++i;
  }
}

bool file_cache::output_changed(const boost::filesystem::path& filename,
                                const crypto::sha256::digest_t& pre_filter_hash)
{
  namespace fs = boost::filesystem;

  auto output = _output_cache.find(filename.generic_string());
  if (output == _output_cache.end())
    return true;
  output->second.checked = true;

  if (std::memcmp(output->second.pre_filter_hash.data(), pre_filter_hash.data(),
                  pre_filter_hash.size()) != 0)
  {
    return true;
  }

  if (!fs::exists(filename))
    return true;
  std::ifstream file;
  file.open(filename.string(), std::ios::in | std::ios_base::binary);
  if (!file.is_open())
  {
    log::error() << "Cannot open existing file " << filename << " for reading.";
    return true;
  }
  file.unsetf(std::ios_base::skipws);
  std::string filtered_content;
  std::copy(std::istream_iterator<char>(file), std::istream_iterator<char>(),
            std::back_inserter(filtered_content));
  file.close();

  // Normalize file content into Unix format.
  boost::replace_all(filtered_content, "\r\n", "\n");

  crypto::sha256::digest_t post_filter_hash{};
  crypto::sha256::hash(post_filter_hash, filtered_content);

  if (output->second.post_filter_hash)
  {
    return std::memcmp(output->second.post_filter_hash->data(),
                       post_filter_hash.data(), post_filter_hash.size()) != 0;
  }
  else
  {
    return std::memcmp(output->second.pre_filter_hash.data(),
                       post_filter_hash.data(), post_filter_hash.size()) != 0;
  }
}

void file_cache::update_cache(
  const boost::filesystem::path& filename,
  const crypto::sha256::digest_t& pre_filter_hash,
  std::shared_ptr<crypto::sha256::digest_t> post_filter_hash)
{
  output_cache_entry newEntry;
  newEntry.pre_filter_hash = pre_filter_hash;
  newEntry.post_filter_hash = std::move(post_filter_hash);
  newEntry.checked = true;
  _output_cache[filename.generic_string()] = std::move(newEntry);
}
}
