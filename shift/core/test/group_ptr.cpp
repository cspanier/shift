#include "probe.h"
#include <shift/core/group_ptr.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <vector>
#include <atomic>
#include <cstdint>

using namespace shift::core;
using probe_t = probe<std::int32_t>;

BOOST_AUTO_TEST_CASE(groupptr_construction)
{
  auto group = std::make_shared<shift::core::group>();
  {
    auto ptr = make_group_ptr<probe_t>(group);
    BOOST_CHECK_EQUAL(probe_t::counter(), 1);
  }
  BOOST_CHECK_EQUAL(probe_t::counter(), 1);

  group.reset();
  BOOST_CHECK_EQUAL(probe_t::counter(), 0);
}

BOOST_AUTO_TEST_CASE(groupptr_copy_assignment)
{
  auto group = std::make_shared<shift::core::group>();
  {
    auto ptr1 = make_group_ptr<probe_t>(group);
    auto ptr2 = ptr1;
    BOOST_CHECK_EQUAL(probe_t::counter(), 1);
  }
  BOOST_CHECK_EQUAL(probe_t::counter(), 1);

  group.reset();
  BOOST_CHECK_EQUAL(probe_t::counter(), 0);
}

BOOST_AUTO_TEST_CASE(groupptr_move_assignment)
{
  auto group = std::make_shared<shift::core::group>();
  {
    auto ptr1 = make_group_ptr<probe_t>(group);
    auto ptr2 = std::move(ptr1);
    BOOST_CHECK_EQUAL(probe_t::counter(), 1);
  }
  BOOST_CHECK_EQUAL(probe_t::counter(), 1);

  group.reset();
  BOOST_CHECK_EQUAL(probe_t::counter(), 0);
}

struct cyclic_dependency
{
  group_ptr<probe<cyclic_dependency>> other;
};

BOOST_AUTO_TEST_CASE(groupptr_cyclic_dependency)
{
  auto group = std::make_shared<shift::core::group>();
  {
    auto ptr1 = make_group_ptr<probe<cyclic_dependency>>(group);
    {
      auto ptr2 = make_group_ptr<probe<cyclic_dependency>>(group);
      ptr1->value().other = ptr2;
      ptr2->value().other = ptr1;
      BOOST_CHECK_EQUAL(probe<cyclic_dependency>::counter(), 2);
    }
    // ptr2 is out of scope, but the instance is referenced by ptr1->other.
    BOOST_CHECK_EQUAL(probe<cyclic_dependency>::counter(), 2);
  }
  // We still have a group instance.
  BOOST_CHECK_EQUAL(probe<cyclic_dependency>::counter(), 2);

  group.reset();
  BOOST_CHECK_EQUAL(probe<cyclic_dependency>::counter(), 0);
}
