#ifndef SHIFT_RC_APPLICATION_HPP
#define SHIFT_RC_APPLICATION_HPP

#include <string>
#include <boost/filesystem/path.hpp>
#include <shift/rc/resource_compiler.hpp>
#include "shift/rc/program_options.hpp"

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
