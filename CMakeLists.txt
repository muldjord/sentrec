cmake_minimum_required(VERSION 3.16)

project(SentRec VERSION 0.0.1 LANGUAGES CXX)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DVERSION=${VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

if(WIN32)
  set(APP_ICON_RESOURCE "win32_icon.rc")
endif()

# Set build type with '-DCMAKE_BUILD_TYPE=type' where type is usually 'Debug' or 'Release'.
# Default to 'Release' type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia LinguistTools)

qt_standard_project_setup()

qt_add_resources(QT_RESOURCES
  sentrec.qrc
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE REQUIRED sndfile)

set(SOURCES
  src/aboutbox.cpp
  src/audiorecorder.cpp
  src/sentencelist.cpp
  src/sentencemodel.cpp
  src/wavhandler.cpp
  src/waveformwidget.cpp
  src/main.cpp
  src/mainwindow.cpp
  src/preferences/configcheckbox.cpp
  src/preferences/configcombobox.cpp
  src/preferences/configdialog.cpp
  src/preferences/configlineedit.cpp
  src/preferences/configpages.cpp
  src/preferences/configpathedit.cpp
  src/preferences/configslider.cpp
  src/preferences/configtextedit.cpp
)

qt_add_executable(SentRec
  ${SOURCES}
  ${QT_RESOURCES}
  ${APP_ICON_RESOURCE}
)
target_include_directories(SentRec PRIVATE ${SNDFILE_INCLUDE_DIRS})
target_link_libraries(SentRec PRIVATE Qt6::Widgets Qt6::Multimedia ${SNDFILE_LIBRARIES})

set_target_properties(SentRec PROPERTIES
  COMPILE_WARNING_AS_ERROR ON
  WIN32_EXECUTABLE ON
)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/debug CACHE PATH "Install path prefix" FORCE)
  else()
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/release CACHE PATH "Install path prefix" FORCE)
  endif()
endif()

# Update translation from sources:
# - Use 'make update_translations' to update ts file
# - Use 'make release_translations' to update qm file
qt_add_translations(SentRec TS_FILES sentrec_da_DK.ts
  SOURCES ${SOURCES}
  # LUPDATE_OPTIONS -no-obsolete
)

add_dependencies(SentRec release_translations)

install(TARGETS SentRec
  RUNTIME DESTINATION .
)

install(FILES
  ${CMAKE_SOURCE_DIR}/AUTHORS
  ${CMAKE_SOURCE_DIR}/LICENSE
  ${CMAKE_SOURCE_DIR}/config.ini.example
  ${CMAKE_CURRENT_BINARY_DIR}/sentrec_da_DK.qm
  ${CMAKE_SOURCE_DIR}/README.md
  DESTINATION .
)
