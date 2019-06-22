# Find shared library version of ZStd
if(NOT ZSTD_SHARED_LIBRARY)
  find_path(ZSTD_SHARED_INCLUDE_DIR "zstd.h")
  find_library(ZSTD_SHARED_LIBRARY_DEBUG
    NAMES
      zstd_d
      zstd
    PATH_SUFFIXES
      lib
  )
  find_library(ZSTD_SHARED_LIBRARY_RELEASE
    NAMES
      zstd
    PATH_SUFFIXES
      lib
  )
  set(ZSTD_SHARED_LIBRARY
    debug ${ZSTD_SHARED_LIBRARY_DEBUG}
    optimized ${ZSTD_SHARED_LIBRARY_RELEASE}
  )
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(ZSTD_SHARED REQUIRED_VARS
  ZSTD_SHARED_LIBRARY ZSTD_SHARED_INCLUDE_DIR)

mark_as_advanced(ZSTD_SHARED_INCLUDE_DIRS)
mark_as_advanced(ZSTD_SHARED_LIBRARIES)

if(ZSTD_SHARED_FOUND)
  set(ZSTD_SHARED_INCLUDE_DIRS ${ZSTD_SHARED_INCLUDE_DIR})
  set(ZSTD_SHARED_LIBRARIES ${ZSTD_SHARED_LIBRARY})

  if(NOT TARGET ZStd::shared)
    add_library(ZStd::shared UNKNOWN IMPORTED)
    set_target_properties(ZStd::shared PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${ZSTD_SHARED_INCLUDE_DIRS}")

    set_property(TARGET ZStd::shared APPEND PROPERTY
      IMPORTED_CONFIGURATIONS RELEASE)
    set_property(TARGET ZStd::shared APPEND PROPERTY
      IMPORTED_CONFIGURATIONS DEBUG)

    if(ZSTD_SHARED_LIBRARY_DEBUG AND ZSTD_SHARED_LIBRARY_RELEASE)
      set_target_properties(ZStd::shared PROPERTIES
        IMPORTED_LOCATION_DEBUG "${ZSTD_SHARED_LIBRARY_DEBUG}"
        IMPORTED_LOCATION_RELEASE "${ZSTD_SHARED_LIBRARY_RELEASE}"
      )
    else()
      set_target_properties(ZStd::shared PROPERTIES
        IMPORTED_LOCATION "${ZSTD_SHARED_LIBRARY_RELEASE}"
      )
    endif()
  endif()
endif()

# Find static library version of ZStd
if(NOT ZSTD_STATIC_LIBRARY)
  find_path(ZSTD_STATIC_INCLUDE_DIR "common/zstd_errors.h")
  find_library(ZSTD_STATIC_LIBRARY_DEBUG
    NAMES
      zstd_static_d
      zstd_static
    PATH_SUFFIXES
      lib
  )
  find_library(ZSTD_STATIC_LIBRARY_RELEASE
    NAMES
      zstd_static
    PATH_SUFFIXES
      lib
  )
  set(ZSTD_STATIC_LIBRARY
    debug ${ZSTD_STATIC_LIBRARY_DEBUG}
    optimized ${ZSTD_STATIC_LIBRARY_RELEASE}
  )
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(ZSTD_STATIC REQUIRED_VARS
  ZSTD_STATIC_LIBRARY ZSTD_STATIC_INCLUDE_DIR)

mark_as_advanced(ZSTD_STATIC_INCLUDE_DIRS)
mark_as_advanced(ZSTD_STATIC_LIBRARIES)

if(ZSTD_STATIC_FOUND)
  set(ZSTD_STATIC_INCLUDE_DIRS ${ZSTD_STATIC_INCLUDE_DIR})
  set(ZSTD_STATIC_LIBRARIES ${ZSTD_STATIC_LIBRARY})

  if(NOT TARGET ZStd::static)
    add_library(ZStd::static UNKNOWN IMPORTED)
    set_target_properties(ZStd::static PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${ZSTD_STATIC_INCLUDE_DIRS}")

    set_property(TARGET ZStd::static APPEND PROPERTY
      IMPORTED_CONFIGURATIONS RELEASE)
    set_property(TARGET ZStd::static APPEND PROPERTY
      IMPORTED_CONFIGURATIONS DEBUG)

    if(ZSTD_STATIC_LIBRARY_DEBUG AND ZSTD_STATIC_LIBRARY_RELEASE)
      set_target_properties(ZStd::static PROPERTIES
        IMPORTED_LOCATION_DEBUG "${ZSTD_STATIC_LIBRARY_DEBUG}"
        IMPORTED_LOCATION_RELEASE "${ZSTD_STATIC_LIBRARY_RELEASE}"
      )
    else()
      set_target_properties(ZStd::static PROPERTIES
        IMPORTED_LOCATION "${ZSTD_STATIC_LIBRARY_RELEASE}"
      )
    endif()
  endif()
endif()
