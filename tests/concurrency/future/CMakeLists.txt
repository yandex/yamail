include_directories(${CMAKE_SOURCE_DIR}/include .)

set(DEPENDENCIES
    ${TARGET_LIBRARY}
    ${LIBS}
    ${GTEST_LIBRARY}
    ${GTEST_MAIN_LIBRARY}
)

file(GLOB_RECURSE SOURCES "*.cc")

add_executable("future-test" ${SOURCES})
target_link_libraries("future-test" ${DEPENDENCIES})
add_test("future-test" "future-test")
