shift_add_library(shift.core STATIC
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  PUBLICSOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATESOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/private
  DEPENDENCIES
    shift.platform
    Boost::boost
)

# install(TARGETS shift.core DESTINATION cmake/shift/core EXPORT shift-core)
# install(EXPORT shift-core DESTINATION cmake/shift)

shift_add_test(test.shift.core
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  SOURCEROOTS
    ${CMAKE_CURRENT_SOURCE_DIR}/test
  DEPENDENCIES
    shift.core
    shift.platform
    Boost::boost
    Boost::unit_test_framework
)
