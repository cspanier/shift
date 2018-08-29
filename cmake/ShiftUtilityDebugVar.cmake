macro(debug_var var_name)
  if (SHIFT_DEBUG)
    message(STATUS "Debug: ${var_name}: \"${${var_name}}\"")
  endif()
endmacro()
