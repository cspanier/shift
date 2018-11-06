#ifndef SHIFT_LIVEDEBUG_LISTENER_IMPL_HPP
#define SHIFT_LIVEDEBUG_LISTENER_IMPL_HPP

#include <vector>
#include <thread>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/livedebug/listener.hpp"

namespace shift::livedebug
{
class debug_server;

// Accepts incoming connections and launches the sessions
class listener_impl : public listener,
                      public std::enable_shared_from_this<listener_impl>
{
public:
  /// Constructor.
  listener_impl(debug_server& server, boost::asio::io_context& io_context,
                boost::asio::ip::tcp::endpoint endpoint);

  ///
  ~listener_impl() override;

  /// Starts accepting incoming connections.
  void run();

  ///
  void close() override;

  ///
  void do_accept();

private:
  ///
  void on_accept(boost::system::error_code error);

  debug_server& _server;
  boost::asio::ip::tcp::acceptor _acceptor;
  boost::asio::ip::tcp::socket _socket;
};
}

#endif
