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
    UNIT_TEST_SOURCES
    DummyTest.cpp
    UnitTestsMain.cpp
)

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    PREFIX "Source Files"
    FILES ${UNIT_TEST_SOURCES}
)

add_executable(
    unit_tests
    ${UNIT_TEST_SOURCES}
)

target_link_libraries(
    unit_tests
    lib.gtest
)

set_target_properties(
    unit_tests
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_test(NAME unit_tests COMMAND unit_tests)

if(DEFINED QT_VERSION)
  add_executable(
      visualizer_mock
      qtMock/visualizer_mock.cpp
  )

  target_include_directories(
      visualizer_mock
      PRIVATE
      qtMock
  )

  add_test(NAME visualizer_mock COMMAND visualizer_mock)
endif()
