cmake_minimum_required (VERSION 2.6)
set(CMAKE_MODULE_PATH 
  ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/"
)

project(yamail)
set(TARGET_LIBRARY "yamail")

set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

set(CMAKE_CPP_FLAGS_RELEASE  "-DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE  " -g -O2")
set(CMAKE_CXX_FLAGS_DEBUG    " -g -O0")

set(FLAGS
 "-Wall"
 "-Wextra"
 "-Wconversion"
 "-Wcast-qual"
 "-Winline"
 "-Woverloaded-virtual"
 "-Wctor-dtor-privacy"
 "-Wnon-virtual-dtor"
 "-Wold-style-cast"
# "-Wno-sign-conversion"
 "-Wpacked"
 "-Wredundant-decls"
 "-fexceptions"
 # You don't need it. Use 'find_package(Threads REQUIRED)' from cmake
 # "-pthread" <- not present in clang
 "-fPIC"
 "-pipe"
)
foreach(FLAG ${FLAGS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
endforeach()

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
add_definitions(-DBOOST_LOG_DYN_LINK)
add_definitions(-DBOOST_LOG_USE_NATIVE_SYSLOG)
find_package(Boost 1.53 REQUIRED COMPONENTS 
    log
    log_setup
    filesystem
    system
    thread
    regex
    chrono
)

find_package(Threads REQUIRED)

include_directories(${INCLUDE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.cc")
add_library(${TARGET_LIBRARY} SHARED ${SOURCES})
target_link_libraries(${TARGET_LIBRARY} 
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)
set_target_properties(${TARGET_LIBRARY} PROPERTIES SOVERSION 0 VERSION 0.0)

if(CMAKE_BUILD_TEST)
    enable_testing()
    add_subdirectory(tests)
endif(CMAKE_BUILD_TEST)

# CREATE PKGCONFIG FILES
# we use the same files like ./configure, so we have to set its vars
if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR lib64)
endif(NOT DEFINED CMAKE_INSTALL_LIBDIR)
set(prefix      ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${CMAKE_INSTALL_PREFIX})
set(libdir      ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
set(includedir  ${CMAKE_INSTALL_PREFIX}/include)
# set(LIBS        "-lz -lm")

install(DIRECTORY ${INCLUDE_DIR}/yamail DESTINATION include)
install(TARGETS ${TARGET_LIBRARY} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
