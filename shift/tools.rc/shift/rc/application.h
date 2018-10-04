#ifndef SHIFT_RC_APPLICATION_H
#define SHIFT_RC_APPLICATION_H

#include <string>
#include <boost/filesystem/path.hpp>
#include <shift/rc/resource_compiler.h>
#include "shift/rc/program_options.h"

namespace shift
{
namespace rc
{
  ///
  class application : public program_options
  {
  public:
    /// Constructor.
    application();

    /// Main application routine.
    int run();

    ///
    void queue_job(boost::filesystem::path filename);

  private:
    resource_compiler _compiler;
  };
}
}

#endif
