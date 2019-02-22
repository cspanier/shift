if(NOT NO_VULKAN AND NOT NO_QT5)
  # shift_find_package(Platform)
  # shift_find_package(Gui)
  # shift_find_package(ZLIB)
  # shift_find_package(PNG)
  # shift_find_package(BZIP2)
  # shift_find_package(GLFW3)
  # shift_find_package(Vulkan)

  # Find includes in corresponding build directories
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
  # Instruct CMake to run moc automatically when needed.
  set(CMAKE_AUTOMOC ON)

  # Old syntax:
  # find_package(Qt5 COMPONENTS Core Qml Quick QuickControls2 REQUIRED)

  # New syntax:
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Qml REQUIRED)
  find_package(Qt5Quick REQUIRED)
  find_package(Qt5QuickControls2 REQUIRED)

  # Overwrite QT5's C++11 feature requirements to avoid adding -std=g++11 to CXX_FLAGS.
  set(CMAKE_CXX_STANDARD 17)

  # Add path to QML files to make QTCreator find our modules.
  set(QML_IMPORT_PATH
      ${CMAKE_INSTALL_PREFIX}/qml
    CACHE string "Qt Creator extra qml import paths" FORCE)

  # These additional variables don't seem to have any effect.
  # set(QML2_IMPORT_PATH ${QML_IMPORT_PATH}
  #   CACHE string "Qt Creator extra qml import paths" FORCE)
  # set(QML_DESIGNER_IMPORT_PATH ""
  #   CACHE string "Qt Creator extra qml import paths" FORCE)

  shift_add_executable(shift.tools.editor
    VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
    SOURCEDIRS
      ${CMAKE_CURRENT_SOURCE_DIR}
    QMLDIRS
      # ToDo: Implement better way to add custom files to target
      ${CMAKE_INSTALL_PREFIX}/icons
      ${CMAKE_INSTALL_PREFIX}/qml
    DEPENDENCIES
      shift.application
      shift.render.vk
      shift.graphics
      shift.scene
      shift.resource_db
      shift.parser.json
      shift.serialization2
      shift.task
      shift.log
      shift.crypto
      shift.core
      shift.platform
      Qt5::QuickControls2
      Qt5::Quick
      Qt5::Qml
      Qt5::Core
      Boost::boost
      Boost::iostreams
      OSLibs::dwmapi
  )
endif()
