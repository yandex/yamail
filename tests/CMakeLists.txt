include(ExternalProject)

enable_testing()

#find_package(Check REQUIRED)
#set(LIBS ${LIBS} ${CHECK_LIBRARIES})
#include_directories(${CHECK_INCLUDE_DIRS})

# Set default ExternalProject root directory
set_directory_properties(PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR}/ThirdParty)

# Add gtest
ExternalProject_Add(
    googletest
    SVN_REPOSITORY http://googletest.googlecode.com/svn/trunk/
    SVN_REVISION -r 660
    TIMEOUT 10
    # Disable install step
    INSTALL_COMMAND ""
    # Wrap download, configure and build steps in a script to log output
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON)

# Specify include dir
ExternalProject_Get_Property(googletest source_dir)
include_directories(${source_dir}/include)

# Add test executable target
add_executable(main-test main.cc)

# Create dependency of MainTest on googletest
add_dependencies(main-test googletest)

# Specify MainTest's link libraries
ExternalProject_Get_Property(googletest binary_dir)
target_link_libraries(
    main-test
    ${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.a
    -pthread)

#include_directories(.)
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
set(LIBS ${LIBS} ${Boost_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTESTS_DIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}\\\"")

add_subdirectory(data)
add_subdirectory(memory)

add_custom_target( check COMMAND main-test )
