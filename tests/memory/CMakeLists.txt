enable_testing()
include_directories(${CMAKE_SOURCE_DIR}/include .)

# we do not use Check by now
# include_directories(${CHECK_INCLUDE_DIRS})
# set(LIBS ${LIBS} ${CHECK_LIBRARIES} isti_clib)
set(DEPENDENCIES 
    ${TARGET_LIBRARY}
    ${LIBS}
    ${GTEST_LIBRARY}
    ${GTEST_MAIN_LIBRARY}
    )

file(GLOB_RECURSE SOURCES "*.cc")

add_executable("memory_test" ${SOURCES})
target_link_libraries("memory_test" ${DEPENDENCIES})
add_test("memory_test" "memory_test")


#foreach(TEST_FILE ${SOURCES})
#    GET_FILENAME_COMPONENT(TEST_NAME ${TEST_FILE} NAME_WE)
#
#    add_executable(${TEST_NAME} ${TEST_FILE})
#    target_link_libraries(${TEST_NAME} ${DEPENDENCIES})
#    add_test(${TEST_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME})
#endforeach()
