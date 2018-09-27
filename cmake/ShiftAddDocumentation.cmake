macro(shift_add_documentation target)
  set(argument_names "DOCDIRS")
  parse_arguments("${argument_names}" "WIN32" "${ARGN}")
  message(STATUS "Adding documentation target ${target}...")

  if(ARG_DOCDIRS)
    set(_docdirs ${ARG_DOCDIRS})
  else()
    set(_docdirs "${CMAKE_CURRENT_SOURCE_DIR}")
  endif()
  find_sources(documentation_files
    ROOTS ${_docdirs}
    EXTS "*.md" "*.txt" "*.png" "*.jpg")

  add_custom_target(${target} ALL
    SOURCES ${documentation_files})
  set_target_folder(${target})

  unset(ARG_DOCDIRS)
endmacro()
