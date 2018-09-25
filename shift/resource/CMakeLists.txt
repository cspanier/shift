shift_add_library(shift.resource STATIC
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  PUBLICSOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATESOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/private
  DEPENDENCIES
    # shift.graphics
    shift.parser.json
    shift.serialization2
    shift.crypto
    shift.math
    shift.core
    shift.platform
    Boost::boost
    BZip2::BZip2
    ZLIB::ZLIB
)
