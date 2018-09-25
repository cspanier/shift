#ifndef SHIFT_LIVEDEBUG_LISTENER_H
#define SHIFT_LIVEDEBUG_LISTENER_H

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
