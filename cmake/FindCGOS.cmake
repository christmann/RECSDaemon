# - Try to find the Congatec CGOS library
# Will define:
#
# CGOS_INCLUDE_DIRS - include directories needed to use the library
# CGOS_LIBRARIES - Libraries to link agains for the driver

INCLUDE(FindPackageHandleStandardArgs)

# use pkg-config as a hint if available
INCLUDE(FindPkgConfig)
IF(PKG_CONFIG_FOUND)
    PKG_CHECK_MODULES(PKG libcgos)
ENDIF()

FIND_PATH(CGOS_INCLUDE_DIRS
          HINTS C:/MinGW/include
           		"C:/Program Files (x86)/MinGW/include"
          		"C:/Program Files/MinGW/include"
                ${PKG_INCLUDE_DIRS}
          NAMES Cgos.h
          PATH_SUFFIXES cgos)

FIND_LIBRARY(CGOS_LIBRARIES
             HINTS C:/MinGW/lib
             	   "C:/Program Files (x86)/MinGW/lib"
          		   "C:/Program Files/MinGW/include/lib"
                   ${PKG_LIBRARY_DIRS}
             NAMES cgos)

# post-process inlude path
IF(CGOS_INCLUDE_DIRS)
    STRING(REGEX REPLACE cgos$$ "" CGOS_INCLUDE_DIRS ${CGOS_INCLUDE_DIRS})
    SET(CGOS_INCLUDE_DIRS ${CGOS_INCLUDE_DIRS} CACHE PATH "cgos include dirs" FORCE)
ENDIF()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(CGOS DEFAULT_MSG CGOS_INCLUDE_DIRS CGOS_LIBRARIES)

# only visible in advanced view
MARK_AS_ADVANCED(CGOS_INCLUDE_DIRS CGOS_LIBRARIES)