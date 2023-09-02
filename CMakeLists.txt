cmake_minimum_required(VERSION 3.15)
project(AGE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
endif()

if(DEFINED QT6_PATH)
  message("-- Qt6 specified at ${QT6_PATH}")
  list(APPEND CMAKE_PREFIX_PATH ${QT6_PATH})
  set(QT_VERSION 6)
elseif(DEFINED QT5_PATH)
  message("-- Qt5 specified at ${QT6_PATH}")
  list(APPEND CMAKE_PREFIX_PATH ${QT5_PATH})
  set(QT_VERSION 5)
else()
  message("-- QTx_PATH not defined, Will search for Qt6 installation")
  find_package(Qt6Widgets QUIET)
  if(Qt6Widgets_FOUND)
    message("-- Found Qt6 system installation")
    set(QT_VERSION 6)
  else()
    message("-- Qt6 installation not found, Will search for Qt5 installation")
    find_package(Qt5Widgets QUIET)
    if(Qt5Widgets_FOUND)
      message("-- Found Qt5 system installation")
      set(QT_VERSION 5)
    else()
      message("-- Failed to find Qt5 installation. No Qt targets will be created")
    endif()
  endif()
endif()

set(
    AGE_CORE_INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/dependencies/cds/include
    ${CMAKE_SOURCE_DIR}/src/core
)

set(
    CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/lang/string/StringRef.cpp
    ${CMAKE_SOURCE_DIR}/src/core/lang/filesystem/PathAwareFstream.cpp
    src/core/intern/QtDefines.hpp
)

add_library(
    lib.core
    STATIC
    ${CORE_SOURCES}
)

target_include_directories(
    lib.core
    PUBLIC
    ${AGE_CORE_INCLUDE_DIRECTORIES}
)

if(DEFINED QT_VERSION)
  find_package(Qt${QT_VERSION} REQUIRED COMPONENTS Core Gui Widgets)
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)
  set(CMAKE_AUTOUIC ON)

  set(
     VISUALIZER_CORE_SOURCES
     ${CMAKE_SOURCE_DIR}/src/visualizer/settings/SettingsRegistry.cpp
  )

  set(
     VISUALIZER_SOURCES
     ${CMAKE_SOURCE_DIR}/src/visualizer/window/VisualizerWindow.cpp
     ${CMAKE_SOURCE_DIR}/src/visualizer/vertex/Vertex.cpp
     ${CMAKE_SOURCE_DIR}/src/visualizer/vertexMenu/VertexMenu.cpp
     ${CMAKE_SOURCE_DIR}/src/visualizer/graphPanel/GraphPanel.cpp
  )

  add_library(
    lib.visualizer_core
    STATIC
    ${VISUALIZER_CORE_SOURCES}
  )

  target_link_libraries(
    lib.visualizer_core
    PUBLIC
    lib.core
  )

  add_executable(
    visualizer
    ${VISUALIZER_SOURCES}
    src/target/visualizer.cpp
  )

  target_link_libraries(
    visualizer
    lib.core
    lib.visualizer_core
    Qt${QT_VERSION}::Core
    Qt${QT_VERSION}::Gui
    Qt${QT_VERSION}::Widgets
  )

  target_include_directories(
    visualizer
    PUBLIC
    src/visualizer
    ${AGE_CORE_INCLUDE_DIRECTORIES}
  )

  target_include_directories(
    lib.visualizer_core
    PUBLIC
    src/visualizer
    ${AGE_CORE_INCLUDE_DIRECTORIES}
  )
endif()

include(FetchContent)

enable_testing()
add_subdirectory(test/unittests)
add_subdirectory(test/metatests)
