#ifndef SHIFT_LIVEDEBUG_LISTENER_HPP
#define SHIFT_LIVEDEBUG_LISTENER_HPP

#include <vector>
#include <thread>

namespace shift::livedebug
{
///
class listener
{
public:
  ///
  virtual ~listener() = 0;

  ///
  virtual void close() = 0;
};
}

#endif
