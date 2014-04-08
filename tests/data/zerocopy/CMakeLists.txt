file( GLOB ZEROCOPY_TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cc" )

add_executable(zerocopy-test EXCLUDE_FROM_ALL ../../main.cc ${ZEROCOPY_TEST_SOURCES} )

add_dependencies(zerocopy-test googletest googlemock)

target_link_libraries( zerocopy-test ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} )