if (BUILD_NO_DEVELOPMENT_VERSION)
  # Get the latest abbreviated commit hash of the working branch
  execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE BUILD_VERSION_GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if("${BUILD_VERSION_GIT_COMMIT_HASH}" STREQUAL "")
    message(WARNING "Failed to retrieve current Git commit hash. You probably don't have Git in your system's PATH.")
    set(BUILD_VERSION_GIT_COMMIT_HASH "<git-not-found>")
  else()
    message(STATUS "Current Git commit hash: ${BUILD_VERSION_GIT_COMMIT_HASH}")
  endif()
else()
  set(BUILD_VERSION_GIT_COMMIT_HASH "<dev-build>")
endif()
