cmake_minimum_required(VERSION 2.8)
project(SensorProviderSystem)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")

set (PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set (PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_INCLUDE_DIR}")

find_package(Log4cxx REQUIRED)
include_directories(${LOG4CXX_INCLUDE_DIRS})

file(GLOB SOURCES src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_link_libraries (${PROJECT_NAME} ${LOG4CXX_LIBRARIES})
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
	target_link_libraries(${PROJECT_NAME} wsock32) # For htonl;
endif ()

INSTALL(PROGRAMS ${CMAKE_BINARY_DIR}/plugins/${CMAKE_SHARED_LIBRARY_PREFIX}${PROJECT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX} DESTINATION plugins)