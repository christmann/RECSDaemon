# - Try to find the log4cxx logging library
# Will define:
#
# LOG4CXX_INCLUDE_DIRS - include directories needed to use the library
# LOG4CXX_LIBRARIES - Libraries to link agains fpr the driver

INCLUDE(FindPackageHandleStandardArgs)

# use pkg-config as a hint if available
INCLUDE(FindPkgConfig)
IF(PKG_CONFIG_FOUND)
    PKG_CHECK_MODULES(PKG liblog4cxx)
ENDIF()

FIND_PATH(LOG4CXX_INCLUDE_DIRS
          HINTS ${LOG4CXX_VISUAL_STUDIO_PROJECT}/src/main/include
          		C:/MinGW/include
          		"C:/Program Files (x86)/MinGW/include"
          		"C:/Program Files/MinGW/include"
                ${PKG_INCLUDE_DIRS}
          NAMES logger.h
          PATH_SUFFIXES log4cxx)

FIND_LIBRARY(LOG4CXX_LIBRARIES
             HINTS ${LOG4CXX_VISUAL_STUDIO_PROJECT}/projects/Debug
                   ${LOG4CXX_VISUAL_STUDIO_PROJECT}/projects/Release
                   C:/MinGW/lib
          		   "C:/Program Files (x86)/MinGW/lib"
          		   "C:/Program Files/MinGW/lib"
                   ${PKG_LIBRARY_DIRS}
             NAMES log4cxx)

if (LOG4CXX_USE_STATIC_LIBS)
	FIND_LIBRARY(APR_LIBRARY
             HINTS ${LOG4CXX_VISUAL_STUDIO_PROJECT}/projects/Debug
                   ${LOG4CXX_VISUAL_STUDIO_PROJECT}/projects/Release
                   C:/MinGW/lib
                           "C:/Program Files (x86)/MinGW/lib"
                           "C:/Program Files/MinGW/lib"
                   ${PKG_LIBRARY_DIRS}
             NAMES apr-1)
	set(LOG4CXX_LIBRARIES ${LOG4CXX_LIBRARIES} ${APR_LIBRARY})

	FIND_LIBRARY(APRUTIL_LIBRARY
             HINTS ${LOG4CXX_VISUAL_STUDIO_PROJECT}/projects/Debug
                   ${LOG4CXX_VISUAL_STUDIO_PROJECT}/projects/Release
                   C:/MinGW/lib
                           "C:/Program Files (x86)/MinGW/lib"
                           "C:/Program Files/MinGW/lib"
                   ${PKG_LIBRARY_DIRS}
             NAMES aprutil-1)
	set(LOG4CXX_LIBRARIES ${LOG4CXX_LIBRARIES} ${APRUTIL_LIBRARY})

        FIND_LIBRARY(EXPAT_LIBRARY
             HINTS ${LOG4CXX_VISUAL_STUDIO_PROJECT}/projects/Debug
                   ${LOG4CXX_VISUAL_STUDIO_PROJECT}/projects/Release
                   C:/MinGW/lib
                           "C:/Program Files (x86)/MinGW/lib"
                           "C:/Program Files/MinGW/lib"
                   ${PKG_LIBRARY_DIRS}
             NAMES expat)
        set(LOG4CXX_LIBRARIES ${LOG4CXX_LIBRARIES} ${EXPAT_LIBRARY})
endif ()

IF ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
	SET(LOG4CXX_LIBRARIES ${LOG4CXX_LIBRARIES} apr-1 aprutil-1)
ENDIF()

# post-process inlude path
IF(LOG4CXX_INCLUDE_DIRS)
    STRING(REGEX REPLACE log4cxx$$ "" LOG4CXX_INCLUDE_DIRS ${LOG4CXX_INCLUDE_DIRS})
    SET(LOG4CXX_INCLUDE_DIRS ${LOG4CXX_INCLUDE_DIRS} CACHE PATH "log4cxx include dirs" FORCE)
ENDIF()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Log4cxx DEFAULT_MSG LOG4CXX_INCLUDE_DIRS LOG4CXX_LIBRARIES)

# only visible in advanced view
MARK_AS_ADVANCED(LOG4CXX_INCLUDE_DIRS LOG4CXX_LIBRARIES)
