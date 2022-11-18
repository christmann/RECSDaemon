message("Setting arm setting in toolchain file")
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR arm)

# specify the cross compiler
set(tools /opt/gcc-linaro-7.5)
set(CMAKE_C_COMPILER ${tools}/bin/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER ${tools}/bin/arm-linux-gnueabihf-g++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -marm" CACHE STRING "c++ flags")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -marm" CACHE STRING "c flags")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -L/lib/arm-linux-gnueabihf/")
LIST(APPEND CMAKE_SYSTEM_LIBRARY_PATH
  /usr/lib/arm-linux-gnueabihf/
)

#include_directories(BEFORE /usr/include/arm-linux-gnueabihf/)

set(CMAKE_FIND_ROOT_PATH ${tools};/)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -idirafter /usr/include/ -idirafter /usr/include/arm-linux-gnueabihf/")