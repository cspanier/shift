include(FindPackageHandleStandardArgs)

find_path(GLFW3_INCLUDE_DIR GLFW/glfw3.h)

find_library(GLFW3_LIBRARY
  NAMES
    glfw3
    glfw
  NAMES_DEBUG
    glfw3d
    glfwd
    # Fallback to release version if debug libraries are not installed
    glfw3
    glfw
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLFW3 REQUIRED_VARS
  GLFW3_LIBRARY GLFW3_INCLUDE_DIR)

mark_as_advanced(GLFW3_INCLUDE_DIR)
mark_as_advanced(GLFW3_LIBRARY)

if(GLFW3_FOUND)
  set(GLFW3_INCLUDE_DIRS ${GLFW3_INCLUDE_DIR})
  set(GLFW3_LIBRARIES ${GLFW3_LIBRARY})
endif()
