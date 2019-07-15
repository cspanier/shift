//#ifndef SHIFT_LIVEDEBUG_WEBSOCKET_SESSION_HPP
//#define SHIFT_LIVEDEBUG_WEBSOCKET_SESSION_HPP

//#include <memory>
//#include <shift/core/boost_disable_warnings.hpp>
//#include <boost/system/error_code.hpp>
//#include <boost/asio/ip/tcp.hpp>
//#include <boost/asio/strand.hpp>
//#include <boost/asio/steady_timer.hpp>
//#include <boost/asio/bind_executor.hpp>
//#include <boost/beast/version.hpp>
//#include <boost/beast/core.hpp>
//#include <boost/beast/websocket.hpp>
//#include <shift/core/boost_restore_warnings.hpp>
//#include "shift/livedebug/session.hpp"

// namespace shift::livedebug
//{
///// Echoes back all received WebSocket messages
// class websocket_session : public session,
//                          public
//                          std::enable_shared_from_this<websocket_session>
//{
// public:
//  /// Take ownership of the socket
//  explicit websocket_session(debug_server& server,
//                             boost::asio::ip::tcp::socket socket);

//  /// Start the asynchronous operation.
//  template <class Body, class Allocator>
//  void run(boost::beast::http::request<
//           Body, boost::beast::http::basic_fields<Allocator>>
//             request)
//  {
//    // Run the timer. The timer is operated
//    // continuously, this simplifies the code.
//    on_timer({});

//    // Set the timer
//    _timer.expires_after(std::chrono::seconds(15));

//    // Accept the websocket handshake
//    _websocket.async_accept(
//      request,
//      boost::asio::bind_executor(
//        _strand, std::bind(&websocket_session::on_accept, shared_from_this(),
//                           std::placeholders::_1)));
//  }

//  ///
//  void send(std::shared_ptr<boost::beast::http::response<
//              boost::beast::http::empty_body>>&& message) override;

//  ///
//  void send(std::shared_ptr<boost::beast::http::response<
//              boost::beast::http::string_body>>&& message) override;

//  ///
//  void send(std::shared_ptr<boost::beast::http::response<
//              boost::beast::http::file_body>>&& message) override;

// private:
//  /// Called when the timer expires.
//  void on_timer(boost::system::error_code error);

//  ///
//  void on_accept(boost::system::error_code error);

//  ///
//  void do_read();

//  ///
//  void on_read(boost::system::error_code error, std::size_t
//  bytes_transferred);

//  ///
//  void on_write(boost::system::error_code error, std::size_t
//  bytes_transferred);

//  boost::beast::websocket::stream<boost::asio::ip::tcp::socket> _websocket;
//  boost::asio::strand<boost::asio::io_context::executor_type> _strand;
//  boost::asio::steady_timer _timer;
//  boost::beast::multi_buffer _buffer;
//};
//}

//#endif
