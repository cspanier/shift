if(NOT SPIRV_CROSS_LIBRARY)
  find_path(SPIRV_CROSS_INCLUDE_DIR "spirv_cross/spirv_cross.hpp")
  find_library(SPIRV_CROSS_LIBRARY_DEBUG
    NAMES
      spirv-cross-core-d
    PATH_SUFFIXES
      lib
  )
  find_library(SPIRV_CROSS_LIBRARY_RELEASE
    NAMES
      spirv-cross-core
    PATH_SUFFIXES
      lib
  )
  set(SPIRV_CROSS_LIBRARY
    debug ${SPIRV_CROSS_LIBRARY_DEBUG}
    optimized ${SPIRV_CROSS_LIBRARY_RELEASE}
  )
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SPIRV_CROSS REQUIRED_VARS
  SPIRV_CROSS_LIBRARY SPIRV_CROSS_INCLUDE_DIR)

mark_as_advanced(SPIRV_CROSS_INCLUDE_DIRS)
mark_as_advanced(SPIRV_CROSS_LIBRARIES)

if(SPIRV_CROSS_FOUND)
  set(SPIRV_CROSS_INCLUDE_DIRS ${SPIRV_CROSS_INCLUDE_DIR})
  set(SPIRV_CROSS_LIBRARIES ${SPIRV_CROSS_LIBRARY})

  if(NOT TARGET SpirVCross::core)
    add_library(SpirVCross::core UNKNOWN IMPORTED)
    set_target_properties(SpirVCross::core PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${SPIRV_CROSS_INCLUDE_DIRS}")

    if(SPIRV_CROSS_LIBRARY_RELEASE)
      set_property(TARGET SpirVCross::core APPEND PROPERTY
        IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties(SpirVCross::core PROPERTIES
        IMPORTED_LOCATION_RELEASE "${SPIRV_CROSS_LIBRARY_RELEASE}")
    endif()
    
    if(SPIRV_CROSS_LIBRARY_DEBUG)
      set_property(TARGET SpirVCross::core APPEND PROPERTY
        IMPORTED_CONFIGURATIONS DEBUG)
      set_target_properties(SpirVCross::core PROPERTIES
        IMPORTED_LOCATION_DEBUG "${SPIRV_CROSS_LIBRARY_DEBUG}")
    endif()
  endif()
endif()
