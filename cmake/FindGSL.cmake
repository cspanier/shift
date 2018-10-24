include(FindPackageHandleStandardArgs)

find_path(GSL_INCLUDE_DIR gsl/gsl)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GSL REQUIRED_VARS GSL_INCLUDE_DIR)

mark_as_advanced(GSL_INCLUDE_DIR)

if(GSL_FOUND)
  set(GSL_INCLUDE_DIRS ${GSL_INCLUDE_DIR})
endif()
