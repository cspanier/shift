#ifndef SHIFT_LIVEDEBUG_REQUEST_HANDLER_HPP
#define SHIFT_LIVEDEBUG_REQUEST_HANDLER_HPP

#include <filesystem>
#include "shift/livedebug/session.hpp"
#include "shift/livedebug/uri.hpp"

namespace shift::livedebug
{
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
    const uri_t& uri, livedebug::session& session) = 0;
};
}

#endif
