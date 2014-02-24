<<<<<<< HEAD

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
=======
enable_testing()
find_package(Check REQUIRED)
set(LIBS ${LIBS} ${CHECK_LIBRARIES})
include_directories(${CHECK_INCLUDE_DIRS})
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GTEST_BOTH_LIBRARIES})

include_directories(.)
#add_executable(test_isti_str test_isti_str.c)
#target_link_libraries(test_isti_str ${LIBS})
#add_test(test_isti_str ${CMAKE_CURRENT_BINARY_DIR}/test_isti_str)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED OFF)
find_package(Boost COMPONENTS system iostreams filesystem timer REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTESTS_DIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}\\\"")
add_subdirectory(data)
>>>>>>> config-parser
