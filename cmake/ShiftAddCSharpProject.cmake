macro(shift_add_csharp_project target)
  message(STATUS "Adding external msproject ${target}...")
  set(projectfile "${CMAKE_CURRENT_SOURCE_DIR}/${target}.csproj")
  if(EXISTS ${projectfile})
    file(READ "${projectfile}" projectfile_content)
    set(projectfile_new_content "${projectfile_content}")

    set(regex_guid "<ProjectGuid>{([0-9A-F]+-[0-9A-F]+-[0-9A-F]+-[0-9A-F]+-[0-9A-F]+)}</ProjectGuid>")
    string(REGEX MATCH "${regex_guid}" project_guid "${projectfile_new_content}")
    string(REGEX REPLACE "${regex_guid}" "\\1" project_guid "${project_guid}")

    set(regex_assembly_name "<AssemblyName>(.*)</AssemblyName>")
    string(REGEX MATCH "${regex_assembly_name}" project_assembly_name "${projectfile_new_content}")
    string(REGEX REPLACE "${regex_assembly_name}" "\\1" project_assembly_name "${project_assembly_name}")
    string(TOLOWER "${target}" target_lower)
    string(TOLOWER "${project_assembly_name}" project_assembly_name_lower)

    set(regex_setings_debug   "'\\$\\(Configuration\\)\\|\\$\\(Platform\\)' == 'Debug\\|AnyCPU'")
    set(regex_setings_profile "'\\$\\(Configuration\\)\\|\\$\\(Platform\\)' == 'Profile\\|AnyCPU'")
    set(regex_setings_release "'\\$\\(Configuration\\)\\|\\$\\(Platform\\)' == 'Release\\|AnyCPU'")
    string(REGEX MATCH ${regex_setings_debug} settings_debug "${projectfile_new_content}")
    string(REGEX MATCH ${regex_setings_profile} settings_profile "${projectfile_new_content}")
    string(REGEX MATCH ${regex_setings_release} settings_release "${projectfile_new_content}")

    if("${settings_debug}" STREQUAL "")
      message(WARNING "C# project file ${projectfile} does not contain a property group for debug builds")
    endif()
    if("${settings_profile}" STREQUAL "")
      message(WARNING "C# project file ${projectfile} does not contain a property group for profile builds")
    endif()
    if("${settings_release}" STREQUAL "")
      message(WARNING "C# project file ${projectfile} does not contain a property group for release builds")
    endif()

    # message(STATUS "${projectfile_new_content}")
    if(NOT "${projectfile_content}" STREQUAL "${projectfile_new_content}")
      message(STATUS "Updated C# project file for target ${target}.")
      file(WRITE "${projectfile}" "${projectfile_new_content}")
    endif()

    # message(STATUS "target=${target}")
    # message(STATUS "GUID=${project_guid}")
    # message(STATUS "assembly name=${project_assembly_name}")

    # if(NOT "${target_lower}" STREQUAL "${project_assembly_name_lower}")
    #   message(WARNING "Assembly name of target ${target} does not match its file name: ${project_assembly_name}")
    # endif()

    set(ENV{CMakeInstallPrefix} "World")
    include_external_msproject(${target}
      "${projectfile}"
      TYPE ${project_guid}
      PLATFORM "Any CPU")
  
    set_target_folder(${target})
  else()
    message(WARNING "C# project file ${projectfile} does not exist")
  endif()
endmacro()
