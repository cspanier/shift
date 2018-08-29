shift_add_library(shift.platform STATIC
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  PUBLICSOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATESOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/private
  INCLUDEDIRS
    ${CMAKE_SOURCE_DIR}/shift/core/public
  DEPENDENCIES
    Boost::boost
    OSLibs::network
    BREAKPAD
)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
  exclude_sources(${CMAKE_CURRENT_SOURCE_DIR}/private/shift/platform/windows/*.cpp)
endif()
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  exclude_sources(${CMAKE_CURRENT_SOURCE_DIR}/private/shift/platform/linux/*.cpp)
endif()
