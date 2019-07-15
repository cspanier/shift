#ifndef SHIFT_LIVEDEBUG_LISTENER_HPP
#define SHIFT_LIVEDEBUG_LISTENER_HPP

#include <vector>
#include <thread>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/livedebug/error_code.hpp"

namespace shift::livedebug
{
class debug_server;

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
public:
  /// Constructor.
  listener(debug_server& server, boost::asio::io_context& io_context);

  ///
  virtual ~listener();

  /// Opens a socket that accepts incoming connections.
  std::error_code open(boost::asio::ip::tcp::endpoint endpoint);

  /// Closes the socket that accepts new connections.
  void close();

  /// Returns the endpoint that was passed to open.
  const boost::asio::ip::tcp::endpoint& endpoint() const;

private:
  ///
  void do_accept();

  ///
  void on_accept(boost::system::error_code error,
                 boost::asio::ip::tcp::socket socket);

  debug_server& _server;
  boost::asio::io_context& _io_context;
  boost::asio::ip::tcp::acceptor _acceptor;
  boost::asio::ip::tcp::endpoint _endpoint;
};
}

#endif
