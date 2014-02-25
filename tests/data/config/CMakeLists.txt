enable_testing()
#find_package(Check REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/include .)
# include_directories(${CHECK_INCLUDE_DIRS})
# set(LIBS ${LIBS} ${CHECK_LIBRARIES} isti_clib)

add_executable(include_opener include_opener.cc)
target_link_libraries(include_opener ${LIBS})
add_test(include_opener ${CMAKE_CURRENT_BINARY_DIR}/include_opener)
