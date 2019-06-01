if(NOT ZSTD_ZSTD_LIBRARY)
  # find_path(ZSTD_ZSTD_INCLUDE_DIR "zstd.h")
  find_path(ZSTD_ZSTD_INCLUDE_DIR "common/zstd_errors.h")
  find_library(ZSTD_ZSTD_LIBRARY_DEBUG
    NAMES
      zstd_static_d.lib  # Windows
      zstd_static.lib    # Windows fallback
      zstd_static_d      # Linux
      zstd_static        # Linux fallback
    PATH_SUFFIXES
      lib
  )
  find_library(ZSTD_ZSTD_LIBRARY_RELEASE
    NAMES
      zstd_static.lib  # Windows
      zstd_static      # Linux
    PATH_SUFFIXES
      lib
  )
  set(ZSTD_ZSTD_LIBRARY
    debug ${ZSTD_ZSTD_LIBRARY_DEBUG}
    optimized ${ZSTD_ZSTD_LIBRARY_RELEASE}
  )
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(ZSTD_ZSTD REQUIRED_VARS
  ZSTD_ZSTD_LIBRARY ZSTD_ZSTD_INCLUDE_DIR)

mark_as_advanced(ZSTD_ZSTD_INCLUDE_DIRS)
mark_as_advanced(ZSTD_ZSTD_LIBRARIES)

if(ZSTD_ZSTD_FOUND)
  set(ZSTD_ZSTD_INCLUDE_DIRS ${ZSTD_ZSTD_INCLUDE_DIR})
  set(ZSTD_ZSTD_LIBRARIES ${ZSTD_ZSTD_LIBRARY})

  if(NOT TARGET ZSTD::ZSTD)
    add_library(ZSTD::ZSTD UNKNOWN IMPORTED)
    set_target_properties(ZSTD::ZSTD PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${ZSTD_ZSTD_INCLUDE_DIRS}")

    set_property(TARGET ZSTD::ZSTD APPEND PROPERTY
      IMPORTED_CONFIGURATIONS RELEASE)
    set_property(TARGET ZSTD::ZSTD APPEND PROPERTY
      IMPORTED_CONFIGURATIONS DEBUG)

    if(ZSTD_ZSTD_LIBRARY_DEBUG AND ZSTD_ZSTD_LIBRARY_RELEASE)
      set_target_properties(ZSTD::ZSTD PROPERTIES
        IMPORTED_LOCATION_DEBUG "${ZSTD_ZSTD_LIBRARY_DEBUG}"
        IMPORTED_LOCATION_RELEASE "${ZSTD_ZSTD_LIBRARY_RELEASE}"
      )
    else()
      set_target_properties(ZSTD::ZSTD PROPERTIES
        IMPORTED_LOCATION "${ZSTD_ZSTD_LIBRARY_RELEASE}"
      )
    endif()
  endif()
endif()
