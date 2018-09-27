if(WIN32)
  set(SHIFT_VERSION_MAJOR 2)
  set(SHIFT_VERSION_MINOR 0)

  add_subdirectory(protogen)
  add_subdirectory(network)
  add_subdirectory(service)
endif()

