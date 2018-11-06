#ifndef SHIFT_LIVEDEBUG_SESSION_HPP
#define SHIFT_LIVEDEBUG_SESSION_HPP

#include <memory>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <shift/core/boost_restore_warnings.hpp>

namespace shift::livedebug
{
class debug_server;

// Base class for client sessions.
class session
{
public:
  /// Constructor.
  explicit session(debug_server& server);

  /// Destructor.
  virtual ~session() = default;

  /// Returns a bad request response.
  static std::shared_ptr<
    boost::beast::http::response<boost::beast::http::string_body>>
  bad_request(boost::beast::string_view why, unsigned int http_version,
              bool http_keep_alive);

  /// Returns a not found response.
  static std::shared_ptr<
    boost::beast::http::response<boost::beast::http::string_body>>
  not_found(boost::beast::string_view target, unsigned int http_version,
            bool http_keep_alive);

  /// Returns a server error response.
  static std::shared_ptr<
    boost::beast::http::response<boost::beast::http::string_body>>
  server_error(boost::beast::string_view what, unsigned int http_version,
               bool http_keep_alive);

  ///
  virtual void send(std::shared_ptr<boost::beast::http::response<
                      boost::beast::http::empty_body>>&& message) = 0;

  ///
  virtual void send(std::shared_ptr<boost::beast::http::response<
                      boost::beast::http::string_body>>&& message) = 0;

  ///
  virtual void send(std::shared_ptr<boost::beast::http::response<
                      boost::beast::http::file_body>>&& message) = 0;

protected:
  debug_server& _server;
};
}

#endif
