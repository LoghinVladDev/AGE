cmake_minimum_required(VERSION 3.15)
project(AEG)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
endif()

if(DEFINED QT6_PATH)
  list(APPEND CMAKE_PREFIX_PATH ${QT6_PATH})
  set(QT_VERSION 6)
elseif(DEFINED QT5_PATH)
  list(APPEND CMAKE_PREFIX_PATH ${QT5_PATH})
  set(QT_VERSION 5)
else()
  message("-- QTx_PATH not defined, Qt Dependant Targets will not be created")
endif()

if(DEFINED QT_VERSION)
  find_package(Qt${QT_VERSION} REQUIRED COMPONENTS Core Gui Widgets)
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)
  set(CMAKE_AUTOUIC ON)

  set(
      VISUALIZER_SOURCES
      src/target/visualizer.cpp
  )

  add_executable(
      visualizer
      ${VISUALIZER_SOURCES}
  )

  target_link_libraries(
      visualizer
      Qt${QT_VERSION}::Core
      Qt${QT_VERSION}::Gui
      Qt${QT_VERSION}::Widgets
  )
endif()

include(FetchContent)

enable_testing()
add_subdirectory(test/unittests)
