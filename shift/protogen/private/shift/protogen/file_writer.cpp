#include "shift/protogen/file_writer.hpp"
#include "shift/protogen/file_cache.hpp"
#include <shift/proto/proto.hpp>
#include <shift/platform/assert.hpp>
#include <shift/log/log.hpp>
#include <fstream>
#include <iterator>
#include <string>
#include <ios>
#include <string>
#include <utility>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/path_traits.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <shift/core/boost_restore_warnings.hpp>

namespace shift::proto
{
std::string file_writer::str() const
{
  return _content.str();
}

const namescope& file_writer::global_scope() const
{
  return *_global_scope;
}

void file_writer::global_scope(const namescope& scope)
{
  _global_scope = &scope;
  if (_current_scope == nullptr)
    _current_scope = _global_scope;
}

const namescope& file_writer::current_scope() const
{
  return *_current_scope;
}

void file_writer::write(boost::filesystem::path filename,
                        boost::filesystem::path clang_format)
{
  namespace fs = boost::filesystem;

  BOOST_ASSERT(_current_scope == _global_scope);

  crypto::sha256::digest_t pre_filter_hash;
  {
    auto normalized_content = _content.str();
    // Normalize file content into Unix format.
    boost::replace_all(normalized_content, "\r\n", "\n");
    crypto::sha256::hash(pre_filter_hash, normalized_content);

    if (!file_cache::singleton_instance().output_changed(filename,
                                                         pre_filter_hash))
      return;
  }

  log::info() << "Writing file " << filename.generic_string();
  {
    auto filepath = filename.parent_path();
    if (!fs::exists(filepath))
      fs::create_directories(filepath);

    std::ofstream file;
    file.open(filename.string(), std::ios::out | std::ios_base::binary);
    if (!file.is_open())
    {
      log::error() << "Cannot open file " << filename.generic_string()
                   << " for writing.";
      return;
    }
    file.unsetf(std::ios_base::skipws);
    file << _content.str();
    file.close();
  }
  if (!clang_format.empty() && fs::exists(clang_format))
  {
    std::system((clang_format.generic_string() + " -style=file -i " +
                 filename.generic_string())
                  .c_str());

    std::ifstream file;
    file.open(filename.string(), std::ios::in | std::ios_base::binary);
    if (!file.is_open())
    {
      log::error() << "Cannot open existing file " << filename.generic_string()
                   << " for reading.";
      return;
    }
    file.unsetf(std::ios_base::skipws);
    std::string filteredContent;
    std::copy(std::istream_iterator<char>(file), std::istream_iterator<char>(),
              std::back_inserter(filteredContent));
    file.close();
    // Normalize file content into Unix format.
    boost::replace_all(filteredContent, "\r\n", "\n");

    auto post_filter_hash = std::make_shared<crypto::sha256::digest_t>();
    crypto::sha256::hash(*post_filter_hash, filteredContent);

    file_cache::singleton_instance().update_cache(filename, pre_filter_hash,
                                                  post_filter_hash);
  }
  else
    file_cache::singleton_instance().update_cache(filename, pre_filter_hash);
}

void file_writer::clear()
{
  _content.str("");
  _content.clear();
}

bool file_writer::switch_namescope(const namescope& target)
{
  if (_current_scope == nullptr)
    return false;
  if (&target == _current_scope)
    return false;

  auto common_root = _current_scope;
  bool found = false;
  while (!found && (common_root != nullptr))
  {
    for (auto target_root = &target; target_root != nullptr;
         target_root = target_root->parent)
    {
      if (target_root == common_root)
      {
        found = true;
        break;
      }
    }
    if (found)
      break;
    common_root = common_root->parent;
  }
  BOOST_ASSERT(found);
  if (!found)
    return false;

  while (_current_scope != common_root)
  {
    if (on_leave_namescope)
      on_leave_namescope(_content, *_current_scope);
    _current_scope = _current_scope->parent;
  }

  while (_current_scope != &target)
  {
    auto next = &target;
    while (next->parent != _current_scope && next->parent != nullptr)
      next = next->parent;
    if (on_enter_namescope)
      on_enter_namescope(_content, *next);
    _current_scope = next;
  }
  return true;
}
}
