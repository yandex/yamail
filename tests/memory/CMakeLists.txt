enable_testing()
include_directories(${CMAKE_SOURCE_DIR}/include .)

# we do not use Check by now
# include_directories(${CHECK_INCLUDE_DIRS})
# set(LIBS ${LIBS} ${CHECK_LIBRARIES} isti_clib)
set(DEPENDENCIES ${LIBS})

file(GLOB_RECURSE SOURCES "*.cc")

foreach(TEST_FILE ${SOURCES})
  GET_FILENAME_COMPONENT(TEST_NAME ${TEST_FILE} NAME_WE)

  add_executable(${TEST_NAME} ${TEST_FILE})
  target_link_libraries(${TEST_NAME} ${DEPENDENCIES})
  add_test(${TEST_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME})
endforeach()
