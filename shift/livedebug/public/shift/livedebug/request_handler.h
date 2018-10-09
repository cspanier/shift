#ifndef SHIFT_LIVEDEBUG_REQUEST_HANDLER_H
#define SHIFT_LIVEDEBUG_REQUEST_HANDLER_H

#include <shift/core/boost_disable_warnings.h>
#include <boost/filesystem/path.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "shift/livedebug/session.h"

namespace shift::livedebug
{
// Return a reasonable mime type based on the extension of a file.
std::string_view mime_type(std::string_view path);

///
class request_handler
{
public:
  /// Destructor.
  virtual ~request_handler() = default;

  /// Request handler.
  /// @return
  ///   Return true to signal that the request has been handled. False indicates
  ///   that the next request handler shall be tried.
  virtual bool operator()(
    const boost::beast::http::request<boost::beast::http::string_body>& request,
    livedebug::session& session) = 0;
};

///
class file_request_handler : public request_handler
{
public:
  file_request_handler(boost::filesystem::path&& path);

  /// Destructor.
  ~file_request_handler() override = default;

  /// @see request_handler::operator().
  bool operator()(
    const boost::beast::http::request<boost::beast::http::string_body>& request,
    livedebug::session& session) override;

private:
  boost::filesystem::path _path;
};
}

#endif
