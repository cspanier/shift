if(NOT SPIRV_CORE_LIBRARY)
  find_path(SPIRV_CORE_INCLUDE_DIR "SPIRV/GlslangToSpv.h")
  find_library(SPIRV_CORE_LIBRARY_DEBUG
    NAMES
      # ToDo: Is there any debug library?
      SPIRV
    PATH_SUFFIXES
      lib
  )
  find_library(SPIRV_CORE_LIBRARY_RELEASE
    NAMES
      SPIRV
    PATH_SUFFIXES
      lib
  )
  set(SPIRV_CORE_LIBRARY
    debug ${SPIRV_CORE_LIBRARY_DEBUG}
    optimized ${SPIRV_CORE_LIBRARY_RELEASE}
  )
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SPIRV_CORE REQUIRED_VARS
  SPIRV_CORE_LIBRARY SPIRV_CORE_INCLUDE_DIR)

mark_as_advanced(SPIRV_CORE_INCLUDE_DIRS)
mark_as_advanced(SPIRV_CORE_LIBRARIES)

if(SPIRV_CORE_FOUND)
  set(SPIRV_CORE_INCLUDE_DIRS ${SPIRV_CORE_INCLUDE_DIR})
  set(SPIRV_CORE_LIBRARIES ${SPIRV_CORE_LIBRARY})

  if(NOT TARGET SpirV::core)
    add_library(SpirV::core UNKNOWN IMPORTED)
    set_target_properties(SpirV::core PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${SPIRV_CORE_INCLUDE_DIRS}")

    set_property(TARGET SpirV::core APPEND PROPERTY
      IMPORTED_CONFIGURATIONS RELEASE)
    set_property(TARGET SpirV::core APPEND PROPERTY
      IMPORTED_CONFIGURATIONS DEBUG)

    if(SPIRV_CORE_LIBRARY_DEBUG AND SPIRV_CORE_LIBRARY_RELEASE)
      set_target_properties(SpirV::core PROPERTIES
        IMPORTED_LOCATION_DEBUG "${SPIRV_CORE_LIBRARY_DEBUG}"
        IMPORTED_LOCATION_RELEASE "${SPIRV_CORE_LIBRARY_RELEASE}"
      )
    else()
      set_target_properties(SpirV::core PROPERTIES
        IMPORTED_LOCATION "${SPIRV_CORE_LIBRARY_RELEASE}"
      )
    endif()
  endif()
endif()