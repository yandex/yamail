include(ExternalProject)

enable_testing()

#find_package(Check REQUIRED)
#set(LIBS ${LIBS} ${CHECK_LIBRARIES})
#include_directories(${CHECK_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unnamed-type-template-args")

# Set default ExternalProject root directory
set_directory_properties(PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR}/ThirdParty)

# Add gtest
ExternalProject_Add(
    googletest
    SVN_REPOSITORY http://googletest.googlecode.com/svn/tags/release-1.7.0
    TIMEOUT 10
    # Disable install step
    INSTALL_COMMAND ""
    # Disable update command, since we use predefined stable version
    UPDATE_COMMAND ""
    # Wrap download, configure and build steps in a script to log output
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON)

ExternalProject_Get_Property(googletest source_dir)
include_directories(${source_dir}/include)

ExternalProject_Get_Property(googletest binary_dir)
set( GTEST_LIBRARIES ${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.a )
set( GTEST_LIBRARY ${GTEST_LIBRARIES})
set( GTEST_MAIN_LIBRARY ${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main.a )


# Add gtest
ExternalProject_Add(
    googlemock
    SVN_REPOSITORY http://googlemock.googlecode.com/svn/tags/release-1.7.0
    TIMEOUT 10
    # Disable install step
    INSTALL_COMMAND ""
    # Disable update command, since we use predefined stable version
    UPDATE_COMMAND ""
    # Wrap download, configure and build steps in a script to log output
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON)

ExternalProject_Get_Property(googlemock source_dir)
include_directories(${source_dir}/include)

ExternalProject_Get_Property(googlemock binary_dir)
set( GMOCK_LIBRARIES ${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}gmock.a )

include_directories("../include")
#
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 
  COMPONENTS system 
             iostreams 
             filesystem 
             timer 
             chrono
             thread 
REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTESTS_DIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}\\\"")

add_subdirectory(data)
add_subdirectory(memory)

add_custom_target( check
    COMMAND zerocopy-test
    COMMAND memory-test )
