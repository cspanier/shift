if(NOT NO_VULKAN)
  shift_add_library(shift.scene STATIC
    VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
    PUBLICSOURCEDIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/public
    PRIVATESOURCEDIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/private
    INCLUDEDIRS
      ${CMAKE_SOURCE_DIR}/shift/math/public
    DEPENDENCIES
      shift.log
      shift.core
      shift.platform
      Boost::boost
      GLFW3
  )
endif()
