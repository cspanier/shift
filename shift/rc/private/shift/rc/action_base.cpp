#include "shift/rc/action_base.h"

namespace shift::rc
{
action_base::action_base(const std::string& action_name,
                         const action_version& action_version,
                         const bool action_support_multithreading)
: _description({action_name, action_version, action_support_multithreading})
{
}

action_base::~action_base() = default;

const std::string& action_base::name() const
{
  return _description.name;
}

const action_version& action_base::version() const
{
  return _description.version;
}

bool action_base::support_multithreading() const
{
  return _description.support_multithreading;
}

void action_base::check_version(const action_version& cached_version)
{
  _modified = _description.version != cached_version;
}

bool action_base::modified() const
{
  return _modified;
}
}
