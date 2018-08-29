#include "shift/network/network_host.h"
#include "shift/network/network_host_impl.h"
#include <shift/platform/network.h>
#include <shift/platform/fpexceptions.h>
#include <shift/platform/environment.h>
#include <shift/log/log.h>
#include <shift/log/log_server.h>
#include <shift/core/core.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/bind.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <tuple>
#include <map>
#include <thread>

namespace shift::network
{
network_host::~network_host()
{
  // Stop and wait for any remaining network threads.
  stop();
}

std::vector<boost::asio::ip::address> network_host::local_interfaces()
{
  using namespace boost::asio::ip;

  std::vector<address> result;

  for (const auto& interface_address : platform::network_interfaces())
  {
    if (interface_address.is_v4())
    {
      auto address4 = interface_address.to_v4();
      if (!address4.is_class_a() && !address4.is_class_b() &&
          !address4.is_class_c())
      {
        continue;
      }
      if (address4 == address_v4::from_string("127.0.0.1"))
        continue;
    }
    else if (interface_address.is_v6())
    {
      auto address6 = interface_address.to_v6();
      if (!address6.is_site_local())
        continue;
    }
    else
      continue;
    result.push_back(interface_address);
  }
  return result;
}

void network_host::start(size_t num_threads)
{
  if (_started)
    return;
  _started = true;

  _num_threads = num_threads;

  _impl->io_context = std::make_unique<boost::asio::io_context>();

  _impl->statistics_update_timer =
    std::make_unique<boost::asio::deadline_timer>(*_impl->io_context);
  _impl->statistics_update_timer->expires_from_now(
    boost::posix_time::seconds(1));
  _impl->last_statistics_update = std::chrono::system_clock::now();
  _impl->statistics_update_timer->async_wait(
    boost::bind(&impl::statistics_update_handler, _impl.get(), _1));

  auto thread_func = [this](size_t thread_id) {
    std::stringstream thread_name;
    thread_name << "NetworkProcessingThread#" << thread_id;
    platform::environment::thread_debug_name(thread_name.str());

    shift::platform::floating_point_exceptions fpexceptions(
      _MCW_EM & ~(_EM_INEXACT | _EM_UNDERFLOW));

    while (!_impl->io_context->stopped())
    {
      try
      {
        _impl->io_context->run();
      }
      catch (boost::exception&)
      {
        log::exception() << "Caught Boost exception";
      }
      catch (std::exception& ex)
      {
        log::exception() << ex.what();
      }
      catch (...)
      {
        log::exception() << "Caught exception of unknown type in "
                            "network processing thread #"
                         << thread_id << ".";
      }
    }
  };
  for (size_t thread_id = 1; thread_id <= _num_threads; ++thread_id)
  {
    _impl->_worker_threads.emplace_back(
      std::make_unique<std::thread>(thread_func, thread_id));
  }
}

bool network_host::running() const
{
  return !_quit;
}

void network_host::stop()
{
  if (!_started)
    return;

  _quit = true;
  {
    std::lock_guard lock(_impl->_incomming_event_queue_mutex);
    // Discard remaining events.
    while (!_impl->_incomming_event_queue.empty())
      _impl->_incomming_event_queue.pop();
    _impl->_incomming_event_signal.notify_all();
  }
  _impl->statistics_update_timer->cancel();

  _impl->io_context->stop();
  for (auto& thread : _impl->_worker_threads)
  {
    try
    {
      if (thread->joinable())
        thread->join();
    }
    catch (std::invalid_argument&)
    {
      // Happens when the thread finishes execution right between the
      // joinable check and the join call. Simply ignore this case.
    }
  }
  _impl->_worker_threads.clear();
  _impl->statistics_update_timer.reset();
  _impl->io_context.reset();
  _started = false;
}

std::size_t network_host::receive(
  std::chrono::system_clock::duration min_timeout,
  std::chrono::system_clock::duration max_timeout)
{
  using namespace std::chrono_literals;

  std::size_t count = 0;
  auto now = std::chrono::system_clock::now();
  if (min_timeout > max_timeout)
    max_timeout = min_timeout;
  auto min_end_time = now + min_timeout;
  auto max_end_time = now + max_timeout;
  auto remaining_time = min_timeout;

  for (;;)
  {
    socket_event event;
    {
      std::unique_lock lock(_impl->_incomming_event_queue_mutex);
      if (!_impl->_incomming_event_signal.wait_for(
            lock, remaining_time,
            [&]() {
              return _quit || !_impl->_incomming_event_queue.empty();
            }) ||
          _quit)
      {
        return count;
      }

      event = std::move(_impl->_incomming_event_queue.front().second);
      _impl->_incomming_event_queue.pop();
      ++count;
    }

    try
    {
      _impl->dispatch(event);
    }
    catch (...)
    {
      // Ignore any exceptions caught here.
    }

    now = std::chrono::system_clock::now();
    if (now < min_end_time)
      remaining_time = min_end_time - now;
    else
    {
      if (now < max_end_time)
        remaining_time = 0ms;
      else
        break;
    }
  }
  return count;
}

void network_host::statistics(statistics_t& statistics) const
{
  statistics.bytes_sent = _impl->bytes_sent;
  statistics.bytes_received = _impl->bytes_received;
  statistics.bytes_sent_per_second = _impl->bytes_sent_per_second;
  statistics.bytes_received_per_second = _impl->bytes_received_per_second;
  statistics.packets_sent = _impl->packets_sent;
  statistics.packets_received = _impl->packets_received;
  statistics.packets_sent_per_second = _impl->packets_sent_per_second;
  statistics.packets_received_per_second = _impl->packets_received_per_second;
}

boost::asio::io_context& network_host::io_context()
{
  BOOST_ASSERT(_impl->io_context);
  return *_impl->io_context;
}

network_host::network_host() : _impl(std::make_unique<impl>())
{
  auto& log_server = log::log_server::singleton_create();
  log_server.configure_channel(network_log::channel,
                               log::channel_severity::debug,
                               log::channel_priority::normal, "network");
  log_server.enable_channel(network_log::channel, true);
}

void network_host::queue_connection_failed(
  std::shared_ptr<tcp_socket_client> socket, boost::system::error_code error)
{
  {
    std::lock_guard lock(_impl->_incomming_event_queue_mutex);
    _impl->_incomming_event_queue.push(
      std::make_pair(std::chrono::system_clock::now(),
                     connect_failed_event{std::move(socket), error}));
  }
  _impl->_incomming_event_signal.notify_one();
}

void network_host::queue_connected(std::shared_ptr<tcp_socket_client> socket)
{
  {
    std::lock_guard lock(_impl->_incomming_event_queue_mutex);
    _impl->_incomming_event_queue.push(std::make_pair(
      std::chrono::system_clock::now(), connected_event{std::move(socket)}));
  }
  _impl->_incomming_event_signal.notify_one();
}

void network_host::queue_closed(std::shared_ptr<tcp_socket_client> socket,
                                boost::system::error_code error)
{
  {
    std::lock_guard lock(_impl->_incomming_event_queue_mutex);
    _impl->_incomming_event_queue.push(
      std::make_pair(std::chrono::system_clock::now(),
                     tcp_socket_closed_event{std::move(socket), error}));
  }
  _impl->_incomming_event_signal.notify_one();
}

void network_host::queue_closed(std::shared_ptr<udp_socket> socket,
                                boost::system::error_code error)
{
  {
    std::lock_guard lock(_impl->_incomming_event_queue_mutex);
    _impl->_incomming_event_queue.push(
      std::make_pair(std::chrono::system_clock::now(),
                     udp_socket_closed_event{std::move(socket), error}));
  }
  _impl->_incomming_event_signal.notify_one();
}

void network_host::queue_packet(std::shared_ptr<tcp_socket_client> socket,
                                std::vector<char> buffer)
{
  {
    std::lock_guard lock(_impl->_incomming_event_queue_mutex);
    _impl->_incomming_event_queue.push(std::make_pair(
      std::chrono::system_clock::now(),
      received_packet_event{std::move(socket), std::move(buffer)}));
  }
  _impl->_incomming_event_signal.notify_one();
  if (on_receive_buffer)
    on_receive_buffer();
}

void network_host::queue_datagram(std::shared_ptr<udp_socket> socket,
                                  std::vector<char> buffer,
                                  boost::asio::ip::udp::endpoint sender)
{
  {
    std::lock_guard lock(_impl->_incomming_event_queue_mutex);
    _impl->_incomming_event_queue.push(std::make_pair(
      std::chrono::system_clock::now(),
      received_datagram_event{std::move(socket), std::move(sender),
                              std::move(buffer)}));
  }
  _impl->_incomming_event_signal.notify_one();
  if (on_receive_buffer)
    on_receive_buffer();
}
}
