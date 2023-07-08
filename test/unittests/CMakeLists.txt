FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG 9b12f74
)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_library(
      lib.gtest
      ${googletest_SOURCE_DIR}/googletest/src/gtest-all.cc
  )
  target_include_directories(
      lib.gtest
      PUBLIC ${googletest_SOURCE_DIR}/googletest/include/
      PRIVATE ${googletest_SOURCE_DIR}/googletest
  )
endif()

set(
    COMMON_INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/src
    ${AGE_CORE_INCLUDE_DIRECTORIES}
)

set(
    UNIT_TEST_SOURCES
    AsyncRunnerTest.cpp
    DummyTest.cpp
    UnitTestsMain.cpp
    GeneratorTest.cpp
    StringRefTest.cpp
)

set(QT_MOCK_INCLUDE)
set(QT_MOCK_LIB)
if(DEFINED QT_VERSION)
  set(QT_MOCK_INCLUDE qtMock)
  set(QT_MOCK_LIB lib.visualizer_core_mock lib.visualizer_core)
  set(CMAKE_AUTOMOC OFF)
  set(CMAKE_AUTORCC OFF)
  set(CMAKE_AUTOUIC OFF)

  set(
      UNIT_TEST_SOURCES
      ${UNIT_TEST_SOURCES}
      qtMock/VisualizerWindowTest.cpp
      SettingsRegistryTest.cpp
  )

  add_executable(
      visualizer_mock
      ${VISUALIZER_SOURCES}
      qtMock/visualizer_mock.cpp
  )

  target_include_directories(
      visualizer_mock
      PUBLIC
      ${CMAKE_SOURCE_DIR}/src/visualizer
      ${QT_MOCK_INCLUDE}
      ${COMMON_INCLUDE_DIRECTORIES}
  )

  set_target_properties(
      visualizer_mock
      PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  )

  target_link_libraries(
      visualizer_mock
      lib.core
      lib.visualizer_core
  )

  add_test(NAME visualizer_mock COMMAND visualizer_mock)

  add_library(
      lib.visualizer_core_mock
      ${VISUALIZER_SOURCES}
  )

  target_include_directories(
      lib.visualizer_core_mock
      PRIVATE
      ${QT_MOCK_INCLUDE}
      ${COMMON_INCLUDE_DIRECTORIES}
  )
endif()

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    PREFIX "Source Files"
    FILES ${UNIT_TEST_SOURCES}
)

add_executable(
    unit_tests
    ${UNIT_TEST_SOURCES}
)

target_include_directories(
    unit_tests PRIVATE ${QT_MOCK_INCLUDE}
    ${COMMON_INCLUDE_DIRECTORIES}
)

target_link_libraries(
    unit_tests
    lib.gtest
    lib.core
    ${QT_MOCK_LIB}
)

set_target_properties(
    unit_tests
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_test(NAME unit_tests COMMAND unit_tests)
