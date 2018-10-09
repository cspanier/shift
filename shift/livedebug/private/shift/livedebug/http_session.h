#ifndef SHIFT_LIVEDEBUG_HTTP_SESSION_H
#define SHIFT_LIVEDEBUG_HTTP_SESSION_H

#include <memory>
#include <shift/core/boost_disable_warnings.h>
#include <boost/system/error_code.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "shift/livedebug/session.h"

namespace shift::livedebug
{
class debug_server;

// Handles an HTTP server connection
class http_session : public session,
                     public std::enable_shared_from_this<http_session>
{
public:
  /// Takes ownership of the socket
  explicit http_session(debug_server& server,
                        boost::asio::ip::tcp::socket socket);

  /// Starts the asynchronous operation
  void run();

  ///
  void do_read();

  ///
  void on_read(boost::system::error_code error, std::size_t bytes_transferred);

  /// @param message
  ///   Stores a type-erased version of the message's shared pointer to keep it
  ///   alive.
  void on_write(boost::system::error_code error, std::size_t bytes_transferred,
                std::shared_ptr<void> message, bool close);

  ///
  void do_close();

  ///
  void send(std::shared_ptr<boost::beast::http::response<
              boost::beast::http::empty_body>>&& message) override;

  ///
  void send(std::shared_ptr<boost::beast::http::response<
              boost::beast::http::string_body>>&& message) override;

  ///
  void send(std::shared_ptr<boost::beast::http::response<
              boost::beast::http::file_body>>&& message) override;

private:
  ///
  template <class Body, class Allocator>
  void do_send(std::shared_ptr<boost::beast::http::response<
                 Body, boost::beast::http::basic_fields<Allocator>>>&& message);

  boost::asio::ip::tcp::socket _socket;
  boost::asio::strand<boost::asio::io_context::executor_type> _strand;
  boost::beast::flat_buffer _buffer;
  boost::beast::http::request<boost::beast::http::string_body> _request;
};
}

#endif
