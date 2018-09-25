shift_add_library(shift.livedebug STATIC
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  PUBLICSOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATESOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/private
  DEPENDENCIES
    shift.network
    shift.serialization
    shift.parser.json
    shift.log
    shift.core
    shift.platform
    Boost::boost
)
