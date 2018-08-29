shift_add_library(shift.application STATIC
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  PUBLICSOURCEDIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/public"
  PRIVATESOURCEDIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/private"
  DEPENDENCIES
    shift.log
    shift.core
    shift.platform
)
