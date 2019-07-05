include(FindPackageHandleStandardArgs)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_path(BREAKPAD_INCLUDE_DIR client/linux/handler/exception_handler.h
    HINTS
      /usr/include/breakpad)
  mark_as_advanced(BREAKPAD_INCLUDE_DIR)

  find_library(BREAKPAD_CLIENT_LIBRARY
    NAMES
      breakpad_client
    HINTS
      /usr/include/breakpad
    PATH_SUFFIXES
      client/linux/)
  mark_as_advanced(BREAKPAD_CLIENT_LIBRARY)

  FIND_PACKAGE_HANDLE_STANDARD_ARGS(Breakpad REQUIRED_VARS
    BREAKPAD_CLIENT_LIBRARY BREAKPAD_INCLUDE_DIR)

  if(BREAKPAD_FOUND)
    set(HAVE_BREAKPAD 1)
    set(BREAKPAD_INCLUDE_DIRS ${BREAKPAD_INCLUDE_DIR})
    set(BREAKPAD_LIBRARIES ${BREAKPAD_CLIENT_LIBRARY})
  endif()

  # Find symbol tools
  set(BREAKPAD_DUMP_SYMS_PATH "/usr/bin/dump_syms")
  if(NOT EXISTS ${BREAKPAD_DUMP_SYMS_PATH} AND Breakpad_REQUIRED)
    message(FATAL_ERROR
      "Cannot find Breakpad tool dump_path in \"${BREAKPAD_DUMP_SYMS_PATH}\".")
  endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(hint_paths)
  foreach(prefix_path ${CMAKE_PREFIX_PATH})
    list(APPEND hint_paths ${prefix_path}/include/breakpad)
  endforeach()
  find_path(BREAKPAD_INCLUDE_DIR client/windows/handler/exception_handler.h
    HINTS
      ${hint_paths})
  mark_as_advanced(BREAKPAD_INCLUDE_DIR)
  unset(hint_paths)

  set(hint_paths_debug)
  set(hint_paths_release)
  foreach(prefix_path ${CMAKE_PREFIX_PATH})
    list(APPEND hint_paths_debug ${prefix_path}/lib/breakpad/debug)
    list(APPEND hint_paths_release ${prefix_path}/lib/breakpad/release)
  endforeach()

  find_library(BREAKPAD_CLIENT_COMMON_LIBRARY_DEBUG
    NAMES
      common
    HINTS
      ${hint_paths_debug})
  find_library(BREAKPAD_CLIENT_COMMON_LIBRARY_RELEASE
    NAMES
      common
    HINTS
      ${hint_paths_release})
  mark_as_advanced(BREAKPAD_CLIENT_COMMON_LIBRARY_DEBUG)
  mark_as_advanced(BREAKPAD_CLIENT_COMMON_LIBRARY_RELEASE)
  set(BREAKPAD_CLIENT_COMMON_LIBRARY
    debug ${BREAKPAD_CLIENT_COMMON_LIBRARY_DEBUG}
    optimized ${BREAKPAD_CLIENT_COMMON_LIBRARY_RELEASE})

  find_library(BREAKPAD_CLIENT_CRASH_GENERATION_CLIENT_LIBRARY_DEBUG
    NAMES
      crash_generation_client
    HINTS
      ${hint_paths_debug})
  find_library(BREAKPAD_CLIENT_CRASH_GENERATION_CLIENT_LIBRARY_RELEASE
    NAMES
      crash_generation_client
    HINTS
      ${hint_paths_release})
  mark_as_advanced(BREAKPAD_CLIENT_CRASH_GENERATION_CLIENT_LIBRARY_DEBUG)
  mark_as_advanced(BREAKPAD_CLIENT_CRASH_GENERATION_CLIENT_LIBRARY_RELEASE)
  set(BREAKPAD_CLIENT_CRASH_GENERATION_CLIENT_LIBRARY
    debug ${BREAKPAD_CLIENT_CRASH_GENERATION_CLIENT_LIBRARY_DEBUG}
    optimized ${BREAKPAD_CLIENT_CRASH_GENERATION_CLIENT_LIBRARY_RELEASE})

  find_library(BREAKPAD_CLIENT_CRASH_GENERATION_SERVER_LIBRARY_DEBUG
    NAMES
      crash_generation_server
    HINTS
      ${hint_paths_debug})
  find_library(BREAKPAD_CLIENT_CRASH_GENERATION_SERVER_LIBRARY_RELEASE
    NAMES
      crash_generation_server
    HINTS
      ${hint_paths_release})
  mark_as_advanced(BREAKPAD_CLIENT_CRASH_GENERATION_SERVER_LIBRARY_DEBUG)
  mark_as_advanced(BREAKPAD_CLIENT_CRASH_GENERATION_SERVER_LIBRARY_RELEASE)
  set(BREAKPAD_CLIENT_CRASH_GENERATION_SERVER_LIBRARY
    debug ${BREAKPAD_CLIENT_CRASH_GENERATION_SERVER_LIBRARY_DEBUG}
    optimized ${BREAKPAD_CLIENT_CRASH_GENERATION_SERVER_LIBRARY_RELEASE})

  find_library(BREAKPAD_CLIENT_EXCEPTION_HANDLER_LIBRARY_DEBUG
    NAMES
      exception_handler
    HINTS
      ${hint_paths_debug})
  find_library(BREAKPAD_CLIENT_EXCEPTION_HANDLER_LIBRARY_RELEASE
    NAMES
      exception_handler
    HINTS
      ${hint_paths_release})
  mark_as_advanced(BREAKPAD_CLIENT_EXCEPTION_HANDLER_LIBRARY_DEBUG)
  mark_as_advanced(BREAKPAD_CLIENT_EXCEPTION_HANDLER_LIBRARY_RELEASE)
  set(BREAKPAD_CLIENT_EXCEPTION_HANDLER_LIBRARY
    debug ${BREAKPAD_CLIENT_EXCEPTION_HANDLER_LIBRARY_DEBUG}
    optimized ${BREAKPAD_CLIENT_EXCEPTION_HANDLER_LIBRARY_RELEASE})
  unset(hint_paths_debug)
  unset(hint_paths_release)

  FIND_PACKAGE_HANDLE_STANDARD_ARGS(Breakpad REQUIRED_VARS
    BREAKPAD_CLIENT_COMMON_LIBRARY
    BREAKPAD_CLIENT_CRASH_GENERATION_CLIENT_LIBRARY
    BREAKPAD_CLIENT_CRASH_GENERATION_SERVER_LIBRARY
    BREAKPAD_CLIENT_EXCEPTION_HANDLER_LIBRARY
    BREAKPAD_INCLUDE_DIR
  )

  if(BREAKPAD_FOUND)
    set(HAVE_BREAKPAD 1)
    set(BREAKPAD_INCLUDE_DIRS ${BREAKPAD_INCLUDE_DIR})
    set(BREAKPAD_LIBRARIES
      "${BREAKPAD_CLIENT_COMMON_LIBRARY}"
      "${BREAKPAD_CLIENT_CRASH_GENERATION_CLIENT_LIBRARY}"
      "${BREAKPAD_CLIENT_CRASH_GENERATION_SERVER_LIBRARY}"
      "${BREAKPAD_CLIENT_EXCEPTION_HANDLER_LIBRARY}"
    )
  endif()
endif()
