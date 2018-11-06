#ifndef SHIFT_RENDER_VK_MEMORY_MANAGER_LIVEDEBUG_HPP
#define SHIFT_RENDER_VK_MEMORY_MANAGER_LIVEDEBUG_HPP

#include <string>
#include <shift/livedebug/request_handler.hpp>

namespace shift::render::vk
{
///
class memory_manager_request_handler : public livedebug::request_handler
{
public:
  /// Constructor.
  memory_manager_request_handler(std::string&& target);

  /// Destructor.
  ~memory_manager_request_handler() override = default;

  /// @see request_handler::operator().
  bool operator()(
    const boost::beast::http::request<boost::beast::http::string_body>& request,
    livedebug::session& session) override;

private:
  std::string _target;
};
}

#endif
