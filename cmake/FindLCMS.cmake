if(NOT LCMS_LCMS2_LIBRARY)
  find_path(LCMS_LCMS2_INCLUDE_DIR "lcms2.h")
  find_library(LCMS_LCMS2_LIBRARY_DEBUG
    NAMES
      lcms2  # Arch Linux
      lcms2_static  # Windows
    PATH_SUFFIXES
      lib
  )
  find_library(LCMS_LCMS2_LIBRARY_RELEASE
    NAMES
      lcms2  # Arch Linux
      lcms2_static  # Windows
    PATH_SUFFIXES
      lib
  )
  set(LCMS_LCMS2_LIBRARY
    debug ${LCMS_LCMS2_LIBRARY_DEBUG}
    optimized ${LCMS_LCMS2_LIBRARY_RELEASE}
  )
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(LCMS_LCMS2 REQUIRED_VARS
  LCMS_LCMS2_LIBRARY LCMS_LCMS2_INCLUDE_DIR)

mark_as_advanced(LCMS_LCMS2_INCLUDE_DIRS)
mark_as_advanced(LCMS_LCMS2_LIBRARIES)

if(LCMS_LCMS2_FOUND)
  set(LCMS_LCMS2_INCLUDE_DIRS ${LCMS_LCMS2_INCLUDE_DIR})
  set(LCMS_LCMS2_LIBRARIES ${LCMS_LCMS2_LIBRARY})

  if(NOT TARGET LCMS::LCMS2)
    add_library(LCMS::LCMS2 UNKNOWN IMPORTED)
    set_target_properties(LCMS::LCMS2 PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${LCMS_LCMS2_INCLUDE_DIRS}")

    set_property(TARGET LCMS::LCMS2 APPEND PROPERTY
      IMPORTED_CONFIGURATIONS RELEASE)
    set_property(TARGET LCMS::LCMS2 APPEND PROPERTY
      IMPORTED_CONFIGURATIONS DEBUG)

    if(LCMS_LCMS2_LIBRARY_DEBUG AND LCMS_LCMS2_LIBRARY_RELEASE)
      set_target_properties(LCMS::LCMS2 PROPERTIES
        IMPORTED_LOCATION_DEBUG "${LCMS_LCMS2_LIBRARY_DEBUG}"
        IMPORTED_LOCATION_RELEASE "${LCMS_LCMS2_LIBRARY_RELEASE}"
      )
    else()
      set_target_properties(LCMS::LCMS2 PROPERTIES
        IMPORTED_LOCATION "${LCMS_LCMS2_LIBRARY_RELEASE}"
      )
    endif()
  endif()
endif()