if(NOT SHADERC_CORE_LIBRARY)
  find_path(SHADERC_CORE_INCLUDE_DIR "shaderc/shaderc.hpp")
  find_library(SHADERC_CORE_LIBRARY_DEBUG
    NAMES
      shadercd  # Windows
      shaderc  # Arch Linux
    PATH_SUFFIXES
      lib
  )
  find_library(SHADERC_CORE_LIBRARY_RELEASE
    NAMES
      shaderc  # Arch Linux and Windows
    PATH_SUFFIXES
      lib
  )
  set(SHADERC_CORE_LIBRARY
    debug ${SHADERC_CORE_LIBRARY_DEBUG}
    optimized ${SHADERC_CORE_LIBRARY_RELEASE}
  )
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SHADERC_CORE REQUIRED_VARS
  SHADERC_CORE_LIBRARY SHADERC_CORE_INCLUDE_DIR)

mark_as_advanced(SHADERC_CORE_INCLUDE_DIRS)
mark_as_advanced(SHADERC_CORE_LIBRARIES)

if(SHADERC_CORE_FOUND)
  set(SHADERC_CORE_INCLUDE_DIRS ${SHADERC_CORE_INCLUDE_DIR})
  set(SHADERC_CORE_LIBRARIES ${SHADERC_CORE_LIBRARY})

  if(NOT TARGET ShaderC::core)
    add_library(ShaderC::core UNKNOWN IMPORTED)
    set_target_properties(ShaderC::core PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${SHADERC_CORE_INCLUDE_DIRS}")

    set_property(TARGET ShaderC::core APPEND PROPERTY
      IMPORTED_CONFIGURATIONS RELEASE)
    set_property(TARGET ShaderC::core APPEND PROPERTY
      IMPORTED_CONFIGURATIONS DEBUG)

    if(SHADERC_CORE_LIBRARY_DEBUG AND SHADERC_CORE_LIBRARY_RELEASE)
      set_target_properties(ShaderC::core PROPERTIES
        IMPORTED_LOCATION_DEBUG "${SHADERC_CORE_LIBRARY_DEBUG}"
        IMPORTED_LOCATION_RELEASE "${SHADERC_CORE_LIBRARY_RELEASE}"
      )
    else()
      set_target_properties(ShaderC::core PROPERTIES
        IMPORTED_LOCATION "${SHADERC_CORE_LIBRARY_RELEASE}"
      )
    endif()
  endif()
endif()

################################################################################

if(NOT SHADERC_UTIL_LIBRARY)
  find_path(SHADERC_UTIL_INCLUDE_DIR "shaderc/shaderc.hpp")
  find_library(SHADERC_UTIL_LIBRARY_DEBUG
    NAMES
      shaderc_utild  # Windows
      shaderc_util  # Arch Linux
    PATH_SUFFIXES
      lib
  )
  find_library(SHADERC_UTIL_LIBRARY_RELEASE
    NAMES
      shaderc_util  # Arch Linux and Windows
    PATH_SUFFIXES
      lib
  )
  set(SHADERC_UTIL_LIBRARY
    debug ${SHADERC_UTIL_LIBRARY_DEBUG}
    optimized ${SHADERC_UTIL_LIBRARY_RELEASE}
  )
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SHADERC_UTIL REQUIRED_VARS
  SHADERC_UTIL_LIBRARY SHADERC_UTIL_INCLUDE_DIR)

mark_as_advanced(SHADERC_UTIL_INCLUDE_DIRS)
mark_as_advanced(SHADERC_UTIL_LIBRARIES)

if(SHADERC_UTIL_FOUND)
  set(SHADERC_UTIL_INCLUDE_DIRS ${SHADERC_UTIL_INCLUDE_DIR})
  set(SHADERC_UTIL_LIBRARIES ${SHADERC_UTIL_LIBRARY})

  if(NOT TARGET ShaderC::util)
    add_library(ShaderC::util UNKNOWN IMPORTED)
    set_target_properties(ShaderC::util PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${SHADERC_UTIL_INCLUDE_DIRS}")

    set_property(TARGET ShaderC::util APPEND PROPERTY
      IMPORTED_CONFIGURATIONS RELEASE)
    set_property(TARGET ShaderC::util APPEND PROPERTY
      IMPORTED_CONFIGURATIONS DEBUG)

    if(SHADERC_UTIL_LIBRARY_DEBUG AND SHADERC_UTIL_LIBRARY_RELEASE)
      set_target_properties(ShaderC::util PROPERTIES
        IMPORTED_LOCATION_DEBUG "${SHADERC_UTIL_LIBRARY_DEBUG}"
        IMPORTED_LOCATION_RELEASE "${SHADERC_UTIL_LIBRARY_RELEASE}"
      )
    else()
      set_target_properties(ShaderC::util PROPERTIES
        IMPORTED_LOCATION "${SHADERC_UTIL_LIBRARY_RELEASE}"
      )
    endif()
  endif()
endif()
