macro(shift_add_test target)
  set(argument_names "ROOT;SOURCEROOTS;SOURCEEXTS;INCLUDEDIRS")
  set(argument_names "${argument_names};DEFINITIONS;LIBRARIES;DEPENDENCIES")
  set(argument_names "${argument_names};QT5MODULES;CXXFLAGS;LFLAGS;VERSION")
  set(argument_names "${argument_names};EXCLUDE_FROM_BUILD")
  parse_arguments("${argument_names}" "" "${ARGN}")

  if(ARG_SOURCEROOTS)
    set(source_roots ${ARG_SOURCEROOTS})
  else()
    set(source_roots ${CMAKE_CURRENT_SOURCE_DIR})
    message(STATUS "SOURCEROOTS (${ARG_SOURCEROOTS}) not set, so falling back to use ${source_roots}...")
  endif()
  find_sources(sources
    ROOTS
      ${source_roots}
    EXTS
      "*.h"
      "*.hpp"
      "*.hxx"
      "*.c"
      "*.cc"
      "*.cpp"
      "*.cxx")

  remove_definitions(-D_MBCS)

  if(NOT "${ARG_VERSION}" STREQUAL "")
    string(REGEX REPLACE "\\." "_" version_underscored "${ARG_VERSION}")
    set(version_tag ".${version_underscored}")
  else()
    set(version_tag "")
  endif()

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

  if(NOT sources)
    message(WARNING "No test cases found in folder(s) ${source_roots}")
  endif()
  message(STATUS "Adding test ${target}...")
  add_executable(${target} ${BUILD_MODE} ${sources})
  target_link_libraries(${target} ${ARG_LIBRARIES})

  foreach(dependency ${ARG_DEPENDENCIES})
    if (TARGET ${dependency})
      add_dependencies(${target} ${dependency})
    endif()
  endforeach()

  get_directory_property(include_dirs INCLUDE_DIRECTORIES)
  list(APPEND include_dirs "${CMAKE_CURRENT_SOURCE_DIR}/public")
  list(APPEND include_dirs "${CMAKE_CURRENT_SOURCE_DIR}/private")
  list(APPEND include_dirs "${CMAKE_CURRENT_SOURCE_DIR}/test")
  list(APPEND include_dirs ${ARG_INCLUDEDIRS})
  list(APPEND include_dirs "${CMAKE_PREFIX_PATH}/include")
  if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    # libc++ include directory are set after qt moc file generation because of
    # missing C++11 support in QT4 moc.
    list(APPEND include_dirs ${LIBCXX_INCLUDE_DIRS})
  endif()

  set(qt5_definitions)
  foreach(qt5module ${ARG_QT5MODULES})
    list(APPEND include_dirs "${Qt5${qt5module}_INCLUDE_DIRS}")
    foreach (qt5_definition ${Qt5${qt5module}_DEFINITIONS})
      STRING(REGEX REPLACE "[-/]D(.*)" "\\1" qt5_definition "${qt5_definition}")
      list(APPEND ARG_DEFINITIONS "${qt5_definition}")
    endforeach()
    target_link_libraries(${target} "${Qt5${qt5module}_LIBRARIES}")
  endforeach()

  set(build_file_suffix "${version_tag}.${SHIFT_SYSTEM_PROCESSOR}.${SHIFT_COMPILER_ACRONYM}")

  set(definitions "${SHIFT_GLOBAL_DEFINITIONS}")
  list(APPEND definitions "${ARG_DEFINITIONS}")
  list(APPEND definitions $<$<CONFIG:Release>:_RELEASE>)
  list(APPEND definitions $<$<CONFIG:Debug>:BUILD_BIN_FOLDER="bin.debug">)
  list(APPEND definitions $<$<CONFIG:MinSizeRel>:BUILD_BIN_FOLDER="bin.minsizerel">)
  list(APPEND definitions $<$<CONFIG:Release>:BUILD_BIN_FOLDER="bin">)
  list(APPEND definitions $<$<CONFIG:RelWithDebInfo>:BUILD_BIN_FOLDER="bin.relwithdeb">)
  list(APPEND definitions BUILD_FILE_SUFFIX="${build_file_suffix}")
  list(APPEND definitions SHIFT_TEST_MODULE_NAME=${target})
  set(LFLAGS "${GLOBAL_LINK_FLAGS}")
  list(APPEND LFLAGS " ${ARG_LFLAGS}")

  set_target_properties(${target} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    PREFIX ""
    INCLUDE_DIRECTORIES "${include_dirs}"
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
      "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/test"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG
      "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/test.debug"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
      "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/test.minsizerel"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE
      "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/test"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
      "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/test.relwithdeb"

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
    add_test(NAME test.${target}
      WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/test"
      COMMAND $<TARGET_FILE:${target}> --log_level=all
    )

    install(TARGETS ${target}
      DESTINATION "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/bin")
  endif()

  unset(version_tag)
  unset(sources)
  unset(ARG_ROOT)
  unset(ARG_SOURCEROOTS)
  unset(ARG_SOURCEEXTS)
  unset(ARG_INCLUDEDIRS)
  unset(ARG_DEFINITIONS)
  unset(ARG_LIBRARIES)
  unset(ARG_DEPENDENCIES)
  unset(ARG_QT5MODULES)
  unset(ARG_CXXFLAGS)
  unset(ARG_LFLAGS)
  unset(ARG_VERSION)
  unset(ARG_EXCLUDE_FROM_BUILD)
endmacro()
