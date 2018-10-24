if(NOT SHADERC_LIBRARY)
  find_path(SHADERC_INCLUDE_DIR "shaderc/shaderc.hpp")
  find_library(SHADERC_LIBRARY_DEBUG
    NAMES
      shaderc_combined_d
    PATH_SUFFIXES
      lib
  )
  find_library(SHADERC_LIBRARY_RELEASE
    NAMES
      shaderc_combined
    PATH_SUFFIXES
      lib
  )
  set(SHADERC_LIBRARY
    debug ${SHADERC_LIBRARY_DEBUG}
    optimized ${SHADERC_LIBRARY_RELEASE}
  )
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SHADERC REQUIRED_VARS
  SHADERC_LIBRARY SHADERC_INCLUDE_DIR)

mark_as_advanced(SHADERC_INCLUDE_DIRS)
mark_as_advanced(SHADERC_LIBRARIES)

if(SHADERC_FOUND)
  set(SHADERC_INCLUDE_DIRS ${SHADERC_INCLUDE_DIR})
  set(SHADERC_LIBRARIES ${SHADERC_LIBRARY})

  if(NOT TARGET ShaderC::ShaderC)
    add_library(ShaderC::ShaderC UNKNOWN IMPORTED)
    set_target_properties(ShaderC::ShaderC PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${SHADERC_INCLUDE_DIRS}")

    if(SHADERC_LIBRARY_RELEASE)
      set_property(TARGET ShaderC::ShaderC APPEND PROPERTY
        IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties(ShaderC::ShaderC PROPERTIES
        IMPORTED_LOCATION_RELEASE "${SHADERC_LIBRARY_RELEASE}")
    endif()
    
    if(SHADERC_LIBRARY_DEBUG)
      set_property(TARGET ShaderC::ShaderC APPEND PROPERTY
        IMPORTED_CONFIGURATIONS DEBUG)
      set_target_properties(ShaderC::ShaderC PROPERTIES
        IMPORTED_LOCATION_DEBUG "${SHADERC_LIBRARY_DEBUG}")
    endif()
  endif()
endif()
