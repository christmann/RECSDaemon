
cmake_minimum_required(VERSION 2.8)
project(RECSDaemon)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")

set (PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set (PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_INCLUDE_DIR}")

if (STATIC_LINKING)
	message(STATUS "Statically linking Log4cxx")
	set(DYNAMIC_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
	set(LOG4CXX_USE_STATIC_LIBS 1)
endif ()

find_package(Log4cxx REQUIRED)
include_directories(${LOG4CXX_INCLUDE_DIRS})

if (STATIC_LINKING)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ${DYNAMIC_FIND_LIBRARY_SUFFIXES})
	unset(LOG4CXX_USE_STATIC_LIBS)
endif ()

find_package(Threads REQUIRED)

if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
	find_package(OpenSSL)
	include_directories(${OPENSSL_INCLUDE_DIRS})
endif ()

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/*.cpp ${PROJECT_SOURCE_DIR}/plugin_framework/*.cpp ${PROJECT_SOURCE_DIR}/network/*.cpp)
add_definitions( -DDAEMON_VERSION_MAJOR=${MAJOR_VERSION} -DDAEMON_VERSION_MINOR=${MINOR_VERSION} -DDAEMON_VERSION_REVISION=${PATCH_VERSION} )
add_executable(${PROJECT_NAME} ${SOURCES})
configure_file(conf/log4cxx.xml ../conf/log4cxx.xml COPYONLY)
configure_file(conf/recsdaemon.ini ../conf/recsdaemon.ini COPYONLY)

target_link_libraries (${PROJECT_NAME} ${LOG4CXX_LIBRARIES})
target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
	target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})
	target_link_libraries (${PROJECT_NAME} ${OPENSSL_LIBRARIES})
else ()
	target_link_libraries(${PROJECT_NAME} wsock32 ws2_32 iphlpapi crypt32)
endif ()

INSTALL(PROGRAMS ${PROJECT_BINARY_DIR}/../${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} DESTINATION .)
INSTALL(FILES ${PROJECT_BINARY_DIR}/../conf/log4cxx.xml ${PROJECT_BINARY_DIR}/../conf/recsdaemon.ini DESTINATION conf)
INSTALL(FILES rcscripts/RECSDaemon.service DESTINATION /lib/systemd/system/)
INSTALL(FILES rcscripts/RECSDaemon.init DESTINATION /etc/init.d/ PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ RENAME RECSDaemon)

unset(LOG4CXX_LIBRARIES)
unset(LOG4CXX_LIBRARIES CACHE)
