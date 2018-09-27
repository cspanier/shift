macro(shift_add_protogen target)
  set(argument_names "PROTO_PATH;META_PATH;CPP_SOURCE_PATH;CPP_INCLUDE_PATH;CPP_NAMESPACE;CPP_NAMING_CONVENTION;CS_PATH;CS_NAMESPACE;CS_USING;DOT_PATH")
  parse_arguments("${argument_names}" "NO_META" "${ARGN}")

  if (NOT CMAKE_CROSSCOMPILING)
    message(STATUS "Adding protogen ${target}...")

    if(WIN32)
      set(CLANG_EXECUTABLE "${CMAKE_SOURCE_DIR}/scripts/clang-format.exe")
    elseif(UNIX)
      if(EXISTS "/usr/bin/clang-format")
        set(CLANG_EXECUTABLE "/usr/bin/clang-format")
      elseif(EXISTS "/usr/local/bin/clang-format")
        set(CLANG_EXECUTABLE "/usr/local/bin/clang-format")
      endif()
    endif()
    if(NOT EXISTS ${CLANG_EXECUTABLE})
      message(FATAL_ERROR "Cannot find clang-format executable (${CLANG_EXECUTABLE})")
    endif()

    set(_CPP_SOURCE_PATH "")
    if (NOT "${ARG_CPP_SOURCE_PATH}" STREQUAL "")
      set(_CPP_SOURCE_PATH "--cpp-source-path" "${ARG_CPP_SOURCE_PATH}")
    endif()
    
    set(_CPP_INCLUDE_PATH "")
    if (NOT "${ARG_CPP_INCLUDE_PATH}" STREQUAL "")
      set(_CPP_INCLUDE_PATH "--cpp-include-path" "${ARG_CPP_INCLUDE_PATH}")
    endif()
    
    set(_CPP_NAMESPACE "")
    if (NOT "${ARG_CPP_NAMESPACE}" STREQUAL "")
      set(_CPP_NAMESPACE "--cpp-namespace" "${ARG_CPP_NAMESPACE}")
    endif()
    
    set(_CPP_NAMING_CONVENTION "")
    if (NOT "${ARG_CPP_NAMING_CONVENTION}" STREQUAL "")
      set(_CPP_NAMING_CONVENTION "--cpp-naming-convention" "${ARG_CPP_NAMING_CONVENTION}")
    endif()

    set(_CS_PATH "")
    if (NOT "${ARG_CS_PATH}" STREQUAL "")
      set(_CS_PATH "--c#-path" "${ARG_CS_PATH}")
    endif()
    
    set(_CS_NAMESPACE "")
    if (NOT "${ARG_CS_NAMESPACE}" STREQUAL "")
      set(_CS_NAMESPACE "--c#-namespace" "${ARG_CS_NAMESPACE}")
    endif()
    
    set(_CS_USINGS "")
    if (NOT "${ARG_CS_USING}" STREQUAL "")
      foreach(CS_USING ${ARG_CS_USING})
        list(APPEND _CS_USINGS "--c#-using" "${CS_USING}")
      endforeach()
    endif()

    set(_DOT_PATH "")
    if (NOT "${ARG_DOT_PATH}" STREQUAL "")
      set(_DOT_PATH "--dot-path" "${ARG_DOT_PATH}")
    endif()

    set(sources "")
    foreach(proto_path ${ARG_PROTO_PATH})
      find_sources(_sources ROOTS ${proto_path} EXTS "*.proto")
      list(APPEND sources ${_sources})
    endforeach()
    if(NOT ARG_NO_META)
      if("${ARG_META_PATH}" STREQUAL "")
        set(ARG_META_PATH ${CMAKE_SOURCE_DIR}/shift/proto/public)
      endif()
      find_sources(_sources ROOTS ${ARG_META_PATH} EXTS "*.proto")
      list(APPEND sources ${_sources})
    endif()

    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${target}.json")
      file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/${target}.json" "")
    endif()
    add_custom_target(
      ${target} ALL
      COMMAND $<TARGET_FILE:shift.protogen>
        "--cache-path" "${CMAKE_CURRENT_SOURCE_DIR}/${target}.json"
        ${_CPP_SOURCE_PATH}
        ${_CPP_INCLUDE_PATH}
        ${_CPP_NAMESPACE}
        ${_CPP_NAMING_CONVENTION}
        "--cpp-clang-format" "${CLANG_EXECUTABLE}"
        ${_CS_PATH}
        ${_CS_NAMESPACE}
        ${_CS_USINGS}
        ${_DOT_PATH}
        "--log-arguments"
        "--no-logfile"
        ${sources}
      SOURCES
        ${sources}
        "${CMAKE_CURRENT_SOURCE_DIR}/${target}.json"
      COMMENT "Generating sagittarius service protocol..."
      VERBATIM
    )
    add_dependencies(${target} shift.protogen)
  else()
    add_custom_target(${target} ALL)
  endif()
  set_target_folder(${target})

  unset(sources)
  unset(_CPP_SOURCE_PATH)
  unset(_CPP_INCLUDE_PATH)
  unset(_CPP_NAMESPACE)
  unset(_CPP_NAMING_CONVENTION)
  unset(_CS_PATH)
  unset(_CS_NAMESPACE)
  unset(_CS_USINGS)
  unset(ARG_NO_META)
  unset(ARG_PROTO_PATH)
  unset(ARG_META_PATH)
  unset(ARG_CPP_SOURCE_PATH)
  unset(ARG_CPP_INCLUDE_PATH)
  unset(ARG_CPP_NAMESPACE)
  unset(ARG_NAMING_CONVENTION)
  unset(ARG_CS_PATH)
  unset(ARG_CS_NAMESPACE)
  unset(ARG_CS_USING)
endmacro()
