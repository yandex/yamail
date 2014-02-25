enable_testing()

#find_package(Check REQUIRED)
#set(LIBS ${LIBS} ${CHECK_LIBRARIES})
#include_directories(${CHECK_INCLUDE_DIRS})

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GTEST_BOTH_LIBRARIES})

include_directories(.)

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
