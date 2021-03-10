# - Try to find the ADLINK SEMA library
# Will define:
#
# SEMA_INCLUDE_DIRS - include directories needed to use the library
# SEMA_LIBRARIES - Libraries to link agains for the driver

INCLUDE(FindPackageHandleStandardArgs)

# use pkg-config as a hint if available
INCLUDE(FindPkgConfig)
IF(PKG_CONFIG_FOUND)
    PKG_CHECK_MODULES(PKG semaeapi)
ENDIF()

FIND_PATH(SEMA_INCLUDE_DIRS
          HINTS C:/MinGW/include
           		"C:/Program Files (x86)/MinGW/include"
          		"C:/Program Files/MinGW/include"
                ${PKG_INCLUDE_DIRS}
          NAMES semaeapi.h
          PATH_SUFFIXES sema)

FIND_LIBRARY(SEMA_LIBRARIES
             HINTS C:/MinGW/lib
             	   "C:/Program Files (x86)/MinGW/lib"
          		   "C:/Program Files/MinGW/include/lib"
                   ${PKG_LIBRARY_DIRS}
             NAMES semaeapi)

# post-process inlude path
IF(SEMA_INCLUDE_DIRS)
    SET(SEMA_INCLUDE_DIRS ${SEMA_INCLUDE_DIRS} CACHE PATH "sema include dirs" FORCE)
ENDIF()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SEMA DEFAULT_MSG SEMA_INCLUDE_DIRS SEMA_LIBRARIES)

# only visible in advanced view
MARK_AS_ADVANCED(SEMA_INCLUDE_DIRS SEMA_LIBRARIES)
