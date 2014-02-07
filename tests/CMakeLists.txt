
file(GLOB_RECURSE SOURCES "./*.cc")

set(DEPENDENCIES
    ${TARGET_LIBRARY} 
    ${Boost_LIBRARIES}
)

foreach(TEST_FILE ${SOURCES})
        GET_FILENAME_COMPONENT(TEST_NAME ${TEST_FILE} NAME_WE)

        add_executable(${TEST_NAME} ${TEST_FILE})
        target_link_libraries(${TEST_NAME} ${DEPENDENCIES})
        add_test(${TEST_NAME} ${TEST_NAME})
endforeach()