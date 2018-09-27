shift_add_library(shift.network STATIC
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  PUBLICSOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATESOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/private
  DEPENDENCIES
    shift.log
    shift.core
    shift.platform
    Boost::boost
    Boost::coroutine
    OSLibs::winsock2
)

shift_add_test(test.shift.network
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  SOURCEROOTS
    ${CMAKE_CURRENT_SOURCE_DIR}/test
  DEPENDENCIES
    shift.network
    shift.log
    shift.core
    shift.platform
    Boost::unit_test_framework
    Boost::filesystem
    Boost::coroutine
)
