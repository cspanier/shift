macro(shift_add_executable target)
  set(argument_names "ROOT;DOCDIRS;SOURCEDIRS;QMLDIRS;SOURCEEXTS;SOURCES;PRECOMPILED_HEADER")
  set(argument_names "${argument_names};INCLUDEDIRS;DEFINITIONS;LIBRARIES")
  set(argument_names "${argument_names};DEPENDENCIES;CXXFLAGS;LFLAGS")
  set(argument_names "${argument_names};VERSION;EXCLUDE_FROM_BUILD")
  parse_arguments("${argument_names}" "WIN32" "${ARGN}")
  message(STATUS "Adding executable ${target}...")

  if(NOT "${ARG_VERSION}" STREQUAL "")
    string(REGEX REPLACE "\\." "_" version_underscored "${ARG_VERSION}")
    set(version_tag ".${version_underscored}")
  else()
    set(version_tag "")
  endif()

  unset(sources)
  if(ARG_DOCDIRS)
    set(_docdirs ${ARG_DOCDIRS})
  else()
    set(_docdirs "${CMAKE_CURRENT_SOURCE_DIR}/doc")
  endif()
  find_sources(documentation_files
    ROOTS ${_docdirs}
    EXTS "*.md" "*.txt" "*.png" "*.jpg"
    GROUPPREFIX "doc/")
  list(APPEND sources ${documentation_files})

  if(ARG_SOURCEDIRS)
    if("${ARG_SOURCEEXTS}" STREQUAL "")
      set(ARG_SOURCEEXTS
        # source files
        "*.h"
        "*.hpp"
        "*.hxx"
        "*.c"
        "*.cc"
        "*.cpp"
        "*.cxx"
        "*.inl"
        # resource files
        "*.rc"
        "*.rc2"
        # image files
        "*.png"
        "*.svg"
        "*.jpg"
        "*.bmp"
        "*.ico")
    endif()
    find_sources(private_sources
      ROOTS ${ARG_SOURCEDIRS}
      EXTS ${ARG_SOURCEEXTS})
    list(APPEND sources ${private_sources})
  else()
    message(FATAL_ERROR "SOURCEDIRS argument not set")
  endif()

  if(ARG_QMLDIRS)
    set(qml_exts
      "*.qml"
      "*.js")
    find_sources(qml_sources
      ROOTS ${ARG_QMLDIRS}
      EXTS ${qml_exts}
      GROUPPREFIX "qml/")
    list(APPEND sources ${qml_sources})
  endif()

  list(APPEND sources ${ARG_SOURCES})

  # ToDo: The following link flag variables are not used. Shall we remove them?
  # message(STATUS "${target} LINK_FLAGS: ${GLOBAL_LINK_FLAGS} ; ${ARG_LFLAGS}")
  # message(STATUS "${target} LINK_FLAGS_DEBUG: ${GLOBAL_LINK_FLAGS_DEBUG}")
  # message(STATUS "${target} LINK_FLAGS_RELEASE: ${GLOBAL_LINK_FLAGS_RELEASE}")

  remove_definitions(-D_MBCS)

  foreach(dependency ${ARG_DEPENDENCIES})
    if (TARGET ${dependency})
      list(APPEND ARG_LIBRARIES ${dependency})
    else()
      if(${dependency}_INCLUDE_DIRS)
        list(APPEND ARG_INCLUDEDIRS ${${dependency}_INCLUDE_DIRS})
      endif()
      if(${dependency}_DEFINITIONS)
        list(APPEND ARG_DEFINITIONS ${${dependency}_DEFINITIONS})
      endif()
      if(${dependency}_LIBRARIES)
        list(APPEND ARG_LIBRARIES ${${dependency}_LIBRARIES})
      endif()
    endif()
  endforeach()

  # ToDo: Only for GCC version before 9.0.
  #       Since version 9.0 libstdc++fs is integrated into libstdc++.
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR
      "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    list(APPEND ARG_LIBRARIES "stdc++fs")
  endif()

  if(ARG_WIN32)
    set(BUILD_MODE WIN32)
  else()
    set(BUILD_MODE "")
  endif()
  add_executable(${target} ${BUILD_MODE} ${sources})
  target_link_libraries(${target} ${ARG_LIBRARIES})

  foreach(dependency ${ARG_DEPENDENCIES})
    if (TARGET ${dependency})
      add_dependencies(${target} ${dependency})
    endif()
  endforeach()

  set(all_include_dirs)
  list(APPEND all_include_dirs ${ARG_SOURCEDIRS})
  list(APPEND all_include_dirs ${ARG_INCLUDEDIRS})
  list(APPEND all_include_dirs "${CMAKE_PREFIX_PATH}/include")

  set(build_file_suffix "${version_tag}.${SHIFT_SYSTEM_PROCESSOR}.${SHIFT_COMPILER_ACRONYM}")

  set(definitions "${SHIFT_GLOBAL_DEFINITIONS}")
  list(APPEND definitions "${ARG_DEFINITIONS}")
  list(APPEND definitions $<$<CONFIG:Release>:_RELEASE>)
  list(APPEND definitions $<$<CONFIG:Debug>:BUILD_BIN_FOLDER="bin.debug">)
  list(APPEND definitions $<$<CONFIG:MinSizeRel>:BUILD_BIN_FOLDER="bin.minsizerel">)
  list(APPEND definitions $<$<CONFIG:Release>:BUILD_BIN_FOLDER="bin">)
  list(APPEND definitions $<$<CONFIG:RelWithDebInfo>:BUILD_BIN_FOLDER="bin.relwithdeb">)
  list(APPEND definitions BUILD_FILE_SUFFIX="${build_file_suffix}")
  set(LFLAGS "${GLOBAL_LINK_FLAGS}")
  list(APPEND LFLAGS " ${ARG_LFLAGS}")

  set_target_properties(${target} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    # PREFIX ""
    INCLUDE_DIRECTORIES "${all_include_dirs}"
    COMPILE_DEFINITIONS  # This property being ignored by MSVC CMake generator.
      "${definitions}"
    COMPILE_DEFINITIONS_DEBUG
      "${definitions};BUILD_CONFIG_DEBUG"
    COMPILE_DEFINITIONS_MINSIZEREL
      "${definitions};BUILD_CONFIG_MINSIZEREL"
    COMPILE_DEFINITIONS_RELEASE
      "${definitions};BUILD_CONFIG_RELEASE"
    COMPILE_DEFINITIONS_RELWITHDEBINFO
      "${definitions};BUILD_CONFIG_RELWITHDEBINFO"
    COMPILE_FLAGS "${ARG_CXXFLAGS}"
    LINK_FLAGS "${LFLAGS}"
    LINK_FLAGS_DEBUG "${GLOBAL_LINK_FLAGS_DEBUG}"
    LINK_FLAGS_MINSIZEREL "${GLOBAL_LINK_FLAGS_MINSIZEREL}"
    LINK_FLAGS_RELEASE "${GLOBAL_LINK_FLAGS_RELEASE}"
    LINK_FLAGS_RELWITHDEBINFO "${GLOBAL_LINK_FLAGS_RELWITHDEBINFO}"
    LINKER_LANGUAGE CXX

    RUNTIME_OUTPUT_DIRECTORY
      "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG
      "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/bin.debug"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
      "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/bin.minsizerel"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE
      "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
      "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/bin.relwithdeb"

    RUNTIME_OUTPUT_NAME
      "${target}${build_file_suffix}"
    RUNTIME_OUTPUT_NAME_DEBUG
      "${target}${build_file_suffix}"
    RUNTIME_OUTPUT_NAME_MINSIZEREL
      "${target}${build_file_suffix}"
    RUNTIME_OUTPUT_NAME_RELEASE
      "${target}${build_file_suffix}"
    RUNTIME_OUTPUT_NAME_RELWITHDEBINFO
      "${target}${build_file_suffix}"

    VS_WINDOWS_TARGET_PLATFORM_VERSION
      "${CMAKE_SYSTEM_VERSION}"
  )

  set_target_folder(${target})

  if(ARG_EXCLUDE_FROM_BUILD)
    exclude_target_from_build(${target})
  else()
    install(TARGETS ${target}
      DESTINATION "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/bin"
      EXPORT "${CMAKE_PROJECT_NAME}")
    install(EXPORT "${CMAKE_PROJECT_NAME}"
      DESTINATION "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/cmake")
  endif()

  if(ARG_PRECOMPILED_HEADER)
    shift_add_precompiled_header(${target} "${sources}" "${ARG_PRECOMPILED_HEADER}")
    unset(ARG_PRECOMPILED_HEADER)
  endif()

  # Setup Google Breakpad toolchain on Linux
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_custom_target(breakpad.${target} ALL
      COMMAND ${CMAKE_SOURCE_DIR}/scripts/breakpad-process-binary.sh
        "$<TARGET_FILE:${target}>"
        "${CMAKE_SOURCE_DIR}"
      # SOURCES
      #   $<TARGET_FILE:${target}>
      COMMENT "Producing Breakpad symbols for target ${target}..."
      VERBATIM
    )
    add_dependencies(breakpad.${target} ${target})
  endif()

  unset(all_include_dirs)
  unset(version_tag)
  unset(sources)
  unset(ARG_ROOT)
  unset(ARG_SOURCEDIRS)
  unset(ARG_SOURCEEXTS)
  unset(ARG_SOURCES)
  unset(ARG_PRECOMPILED_HEADER)
  unset(ARG_INCLUDEDIRS)
  unset(ARG_DEFINITIONS)
  unset(ARG_LIBRARIES)
  unset(ARG_DEPENDENCIES)
  unset(ARG_CXXFLAGS)
  unset(ARG_LFLAGS)
  unset(ARG_VERSION)
  unset(ARG_EXCLUDE_FROM_BUILD)
endmacro()
