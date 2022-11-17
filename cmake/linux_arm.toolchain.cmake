message("Setting arm setting in toolchain file")
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR arm)

# specify the cross compiler
SET(CMAKE_C_COMPILER   /opt/arm-gnu-toolchain-11.3-armhf/bin/arm-none-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER /opt/arm-gnu-toolchain-11.3-armhf/bin/arm-none-linux-gnueabihf-g++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -marm" CACHE STRING "c++ flags")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -marm" CACHE STRING "c flags")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -L/lib/arm-linux-gnueabihf/")
LIST(APPEND CMAKE_SYSTEM_LIBRARY_PATH
  /usr/lib/arm-linux-gnueabihf/
)

include_directories("/usr/include/arm-linux-gnueabihf/")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
