#ifndef SHIFT_LIVEDEBUG_FILE_REQUEST_HANDLER_HPP
#define SHIFT_LIVEDEBUG_FILE_REQUEST_HANDLER_HPP

#include <filesystem>
#include "shift/livedebug/request_handler.hpp"

namespace shift::livedebug
{
// Return a reasonable mime type based on the extension of a file.
std::string_view mime_type(std::string_view path);

///
class file_request_handler : public request_handler
{
public:
  file_request_handler(std::filesystem::path&& root_path);

  /// Destructor.
  ~file_request_handler() override = default;

  /// @see request_handler::operator().
  bool operator()(
    const boost::beast::http::request<boost::beast::http::string_body>& request,
    const uri_t& uri, livedebug::session& session) override;

private:
  std::filesystem::path _root_path;
};
}

#endif
