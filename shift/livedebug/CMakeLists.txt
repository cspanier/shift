shift_add_library(shift.livedebug STATIC
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  PUBLICSOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATESOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/private
  DEPENDENCIES
    shift.network
    shift.serialization2
    shift.parser.json
    shift.log
    shift.core
    shift.platform
    Boost::boost
)

shift_add_test(test.shift.livedebug
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  SOURCEROOTS
    ${CMAKE_CURRENT_SOURCE_DIR}/test
  DEPENDENCIES
    shift.application
    shift.livedebug
    shift.serialization2
    shift.parser.json
    shift.log
    shift.crypto
    shift.task
    shift.math
    shift.core
    shift.platform
    Boost::boost
    Boost::unit_test_framework
)
