#ifndef SHIFT_PROTO_GEN_FILECACHE_H
#define SHIFT_PROTO_GEN_FILECACHE_H

#include <memory>
#include <map>
#include <string>
#include <shift/core/boost_disable_warnings.h>
#include <boost/filesystem/path.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <shift/core/singleton.h>
#include <shift/core/exception.h>
#include <shift/crypto/sha256.h>

namespace shift::proto
{
///
struct output_cache_entry
{
  crypto::sha256::digest_t pre_filter_hash;
  std::shared_ptr<crypto::sha256::digest_t> post_filter_hash;
  bool checked = false;
};

///
class file_cache : public core::singleton<file_cache, core::create::on_stack>
{
public:
  /// Sets a base path which will be used to truncate all filenames.
  void base_path(boost::filesystem::path base_path);

  /// Read the json cache file into memory
  bool read_cache(const boost::filesystem::path& cache_filename);

  ///
  bool write_cache(const boost::filesystem::path& cache_filename,
                   const crypto::sha256::digest_t& combined_input_hash);

  /// Compares an input file hash with the stored one.
  bool check_input_hash(const crypto::sha256::digest_t& combined_input_hash);

  /// Checks whether any output files have been changed.
  bool check_output_hashes();

  /// Unlinks (aka deletes) all obsolete output files.
  void unlink_unchecked_files();

  ///
  bool output_changed(const boost::filesystem::path& filename,
                      const crypto::sha256::digest_t& pre_filter_hash);

  ///
  void update_cache(
    const boost::filesystem::path& filename,
    const crypto::sha256::digest_t& pre_filter_hash,
    std::shared_ptr<crypto::sha256::digest_t> post_filter_hash = nullptr);

private:
  boost::filesystem::path _base_path;
  crypto::sha256::digest_t _combined_input_hash;
  std::map<std::string, output_cache_entry> _output_cache;
};
}

#endif
