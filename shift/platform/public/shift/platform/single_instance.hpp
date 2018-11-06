#ifndef SHIFT_PLATFORM_SINGLEINSTANCE_HPP
#define SHIFT_PLATFORM_SINGLEINSTANCE_HPP

#include <memory>

namespace shift::platform
{
class single_instance
{
public:
  ///
  single_instance();

  ///
  ~single_instance();

  ///
  bool check();

private:
  struct impl;

  std::unique_ptr<impl> _impl;
};
}

#endif
