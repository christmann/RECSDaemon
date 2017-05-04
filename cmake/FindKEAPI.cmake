# - Try to find the Kontron KEAPI library
# Will define:
#
# KEAPI_INCLUDE_DIRS - include directories needed to use the library
# KEAPI_LIBRARIES - Libraries to link agains for the driver

INCLUDE(FindPackageHandleStandardArgs)

# use pkg-config as a hint if available
INCLUDE(FindPkgConfig)
IF(PKG_CONFIG_FOUND)
    PKG_CHECK_MODULES(PKG libkeapi)
ENDIF()

FIND_PATH(KEAPI_INCLUDE_DIRS
          HINTS C:/MinGW/include
           		"C:/Program Files (x86)/MinGW/include"
          		"C:/Program Files/MinGW/include"
                ${PKG_INCLUDE_DIRS}
          NAMES keapi.h
          PATH_SUFFIXES keapi)

FIND_LIBRARY(KEAPI_LIBRARIES
             HINTS C:/MinGW/lib
             	   "C:/Program Files (x86)/MinGW/lib"
          		   "C:/Program Files/MinGW/include/lib"
                   ${PKG_LIBRARY_DIRS}
             NAMES keapi)

# post-process inlude path
IF(KEAPI_INCLUDE_DIRS)
    STRING(REGEX REPLACE keapi$$ "" KEAPI_INCLUDE_DIRS ${KEAPI_INCLUDE_DIRS})
    SET(KEAPI_INCLUDE_DIRS ${KEAPI_INCLUDE_DIRS} CACHE PATH "KEAPI include dirs" FORCE)
ENDIF()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(KEAPI DEFAULT_MSG KEAPI_INCLUDE_DIRS KEAPI_LIBRARIES)

# only visible in advanced view
MARK_AS_ADVANCED(KEAPI_INCLUDE_DIRS KEAPI_LIBRARIES)