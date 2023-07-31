set(
    META_TEST_SOURCES
    ConceptsTest.cpp
)

add_library(
    meta_tests
    ${META_TEST_SOURCES}
)

target_include_directories(
    meta_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${AGE_CORE_INCLUDE_DIRECTORIES}
)
