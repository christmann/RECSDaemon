message("Setting arm setting in toolchain file")
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR arm)

# specify the cross compiler
SET(CMAKE_C_COMPILER   /opt/gcc-linaro/bin/arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER /opt/gcc-linaro/bin/arm-linux-gnueabihf-g++)

LIST(APPEND CMAKE_SYSTEM_LIBRARY_PATH
  /usr/lib/arm-linux-gnueabihf/
)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)