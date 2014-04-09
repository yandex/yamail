enable_testing()
include_directories(${CMAKE_SOURCE_DIR}/include .)


set(DEPENDENCIES 
    ${TARGET_LIBRARY}
    ${LIBS}
    ${GTEST_LIBRARY}
    ${GTEST_MAIN_LIBRARY}
    )

file(GLOB_RECURSE SOURCES "*.cc")

add_executable("memory-test" ${SOURCES})
target_link_libraries("memory-test" ${DEPENDENCIES})
add_test("memory-test" "memory-test")

