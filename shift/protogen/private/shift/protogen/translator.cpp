#include "shift/protogen/translator.hpp"
#include "shift/protogen/generator/proto_generator.hpp"
#include "shift/protogen/generator/cpp_generator.hpp"
#include "shift/protogen/generator/cs_generator.hpp"
#include "shift/protogen/generator/dot_generator.hpp"
#include <shift/log/log.hpp>
#include <shift/log/log_server.hpp>
#include <shift/core/string_util.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/any.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <algorithm>

namespace shift::proto
{
translator::translator() = default;

int translator::run()
{
  namespace fs = boost::filesystem;

  int result = EXIT_SUCCESS;

  _generators["proto"] = std::make_unique<generator::proto_generator>();
  _generators["c++"] = std::make_unique<generator::cpp_generator>();
  _generators["c#"] = std::make_unique<generator::cs_generator>();
  _generators["dot"] = std::make_unique<generator::dot_generator>();

  if (_generators.empty())
  {
    log::error() << "No code generator registered.";
    return EXIT_FAILURE;
  }

  bool has_active_generator = false;
  fs::path common_base_path;
  for (auto& generator : _generators)
  {
    auto output_paths = generator.second->output_paths();
    for (const auto& output_path : output_paths)
    {
      if (!output_path.empty())
      {
        if (!fs::exists(output_path))
        {
          if (!fs::create_directories(output_path))
          {
            log::error() << "Cannot create output path " << output_path << ".";
            return EXIT_FAILURE;
          }
        }
        if (!fs::is_directory(output_path))
        {
          log::error() << "Output path " << output_path << " is no directory.";
          return EXIT_FAILURE;
        }
        // Compute common base bath.
        if (common_base_path.empty())
          common_base_path = output_path;
        else
        {
          fs::path base_path;
          auto folder1 = common_base_path.begin();
          auto folder2 = output_path.begin();
          for (; folder1 != common_base_path.end() &&
                 folder2 != output_path.end() && *folder1 == *folder2;
               ++folder1, ++folder2)
          {
            base_path /= *folder1;
          }
          common_base_path = base_path;
        }
      }
      has_active_generator = true;
    }
  }
  if (!has_active_generator)
  {
    log::error() << "No code generator selected.";
    return EXIT_FAILURE;
  }
  if (proto_sources.empty())
  {
    log::error() << "No proto sources specified.";
    return EXIT_FAILURE;
  }
  _file_cache.base_path(common_base_path);
  bool has_cache_file = _file_cache.read_cache(cache_filename);

  namescope global_scope;
  struct File
  {
    File() : hash({})
    {
    }

    std::string name;
    std::string content;
    crypto::sha256::digest_t hash;
  };
  std::vector<File> files;
  crypto::sha256::digest_t combined_input_hash{};
  for (const auto& filePath : proto_sources)
  {
    if (!fs::exists(filePath) || !fs::is_regular_file(filePath))
    {
      log::error() << "The specified source file " << filePath
                   << " either does not exist or is no regular file.";
      return EXIT_FAILURE;
    }

    File file;
    file.name = filePath.generic_string();

    std::ifstream input_file(file.name,
                             std::ios_base::in | std::ios_base::binary);
    if (!input_file.is_open())
    {
      log::error() << "Cannot open file " << file.name << " for reading.";
      result = EXIT_FAILURE;
      continue;
    }
    input_file.unsetf(std::ios_base::skipws);
    std::copy(std::istream_iterator<char>(input_file),
              std::istream_iterator<char>(), std::back_inserter(file.content));
    input_file.close();

    if (file.content.empty())
    {
      log::error() << "File " << file.name << " was empty.";
      continue;
    }
    // Normalize file content into Unix format.
    boost::replace_all(file.content, "\r\n", "\n");

    // Apply replacement of key-value pairs passed by command line.
    for (auto& definition : definitions)
    {
      std::stringstream search_string;
      search_string << "${" << definition.first << "}";
      auto& value = definition.second;
      boost::replace_all(file.content, search_string.str(), value);
    }

    crypto::sha256 context(file.hash);
    context << file.content;
    crypto::combine(combined_input_hash, context.finalize());

    files.push_back(file);
  }

  bool input_unchanged = _file_cache.check_input_hash(combined_input_hash);
  bool output_unchanged = _file_cache.check_output_hashes();

  if (!force_rewrite && has_cache_file && input_unchanged && output_unchanged)
  {
    log::info() << "Skipping code generation.";
    return EXIT_SUCCESS;
  }
  log::info() << "Running code generation.";

#if defined(DEBUG)
  log::warning() << "Performance Warning: The protogen parser might take "
                    "a long time in debug builds.";
#endif

  for (const auto& file : files)
  {
    log::info() << "Parsing file " << file.name << "...";
    try
    {
      global_scope.parse(file.content);
    }
    catch (proto::parse_error& error)
    {
      log::error log;
      log << "Parsing file " << file.name;
      if (const std::size_t* line =
            boost::get_error_info<proto::parse_error_line>(error))
      {
        log << ":" << *line;
      }
      log << " failed:";
      if (const std::string* message =
            boost::get_error_info<proto::parse_error_message>(error))
      {
        log << " " << *message;
      }
      if (const std::string* source =
            boost::get_error_info<proto::parse_error_source>(error))
      {
        log << "\n" << *source;
        if (const std::size_t* column =
              boost::get_error_info<proto::parse_error_column>(error))
        {
          log << "\n" << std::string(*column - 1, ' ') << "^";
        }
      }

      result = EXIT_FAILURE;
    }

    global_scope.symbol_source(file.name);
  }

  if (result != EXIT_SUCCESS)
    return result;

  // Merge all name scopes with the same name.
  global_scope.merge();

  // Convert std::vector<Type> to std::vector<std::shared_ptr<Type>>.
  global_scope.convert_type_vectors();

  // Unpack all nested name scopes into a more handy structure.
  global_scope.unpack();

  for (const auto& exclude_namescope : exclude_namescopes)
  {
    // Find name-scope to exclude.
    auto exclude_namescope_path =
      core::split(exclude_namescope, [](const char c) { return c == '.'; });
    auto* exclude_scope = &global_scope;
    for (const auto& namescope : exclude_namescope_path)
    {
      bool found = false;
      for (auto* child_scope : exclude_scope->namescopes)
      {
        if (child_scope->name == namescope)
        {
          exclude_scope = child_scope;
          found = true;
          break;
        }
      }
      if (!found)
      {
        log::error() << "Cannot find name-scope \"" << namescope
                     << "\", which was passed using the --exclude parameter.";
        return EXIT_FAILURE;
      }
    }
    BOOST_ASSERT(exclude_scope);
    auto& parent_namescopes = exclude_scope->parent->namescopes;
    parent_namescopes.erase(
      std::remove_if(parent_namescopes.begin(), parent_namescopes.end(),
                     [&](namescope* child) { return child == exclude_scope; }),
      parent_namescopes.end());
  }

  // Sort all elements to get stable results.
  global_scope.sort();

  // Semantically check the collection of all parsed documents.
  log::info() << "Performing semantic analysis...";
  if (!global_scope.analyze())
  {
    log::error() << "Semantic analysis failed!";
    return EXIT_FAILURE;
  }
  if (verbose)
    log::info() << "Parsing succeeded.";

  // Compute unique IDs.
  global_scope.generate_uids();

  // Find root name-scope to use for code generation.
  auto strip_namescope_path =
    core::split(strip_namescopes, [](const char c) { return c == '.'; });
  auto* root_scope = &global_scope;
  for (const auto& namescope : strip_namescope_path)
  {
    bool found = false;
    for (auto* child_scope : root_scope->namescopes)
    {
      if (child_scope->name == namescope)
      {
        root_scope = child_scope;
        found = true;
        break;
      }
    }
    if (!found)
    {
      log::error() << "Cannot find name-scope \"" << namescope
                   << "\", which was passed using the --strip parameter.";
      return EXIT_FAILURE;
    }
  }
  BOOST_ASSERT(root_scope);
  root_scope->parent = nullptr;

  // Find root name-scope to use for code generation.
  auto limit_namescope_path =
    core::split(limit_namescopes, [](const char c) { return c == '.'; });
  auto* limit_scope = root_scope;
  for (const auto& namescope : limit_namescope_path)
  {
    bool found = false;
    for (auto* child_scope : limit_scope->namescopes)
    {
      if (child_scope->name == namescope)
      {
        limit_scope = child_scope;
        found = true;
        break;
      }
    }
    if (!found)
    {
      log::error() << "Cannot find name-scope \"" << namescope
                   << "\", which was passed using the --limit parameter.";
      return EXIT_FAILURE;
    }
  }
  BOOST_ASSERT(limit_scope);

  for (const auto& generator : _generators)
  {
    if (generator.second->active())
    {
      log::info() << "Generating " << generator.first << " code...";
      generator.second->generate(*root_scope, *limit_scope);
    }
  }

  _file_cache.unlink_unchecked_files();
  if (_file_cache.write_cache(cache_filename, combined_input_hash))
    log::info() << "Cache file written.";
  else
  {
    log::warning() << "Could not write cache file " << cache_filename << ".";
  }

  return EXIT_SUCCESS;
}
}
