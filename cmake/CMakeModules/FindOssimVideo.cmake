# - Find ossimPredator library
# Find the native ossimPredator includes and library
# This module defines
#  OSSIMVIDEO_INCLUDE_DIR, where to find tiff.h, etc.
#  OSSIMVIDEO_LIBRARIES, libraries to link against to use ossimPredator.
#  OSSIMVIDEO_FOUND, If false, do not try to use ossimPredator.
# also defined, but not for general use are
#  OSSIMVIDEO_LIBRARY, where to find the ossimPredator library.
SET(CMAKE_FIND_FRAMEWORK "LAST")
FIND_PATH(OSSIMVIDEO_INCLUDE_DIR ossimPredator/ossimPredatorExport.h
	HINTS 
		$ENV{OSSIM_DEV_HOME}/ossim-video/include
		$ENV{OSSIM_DEV_HOME}/ossim-video/lib
		$ENV{OSSIM_INSTALL_PREFIX}/include
	PATHS
		$ENV{OSSIM_DEV_HOME}/ossim
		$ENV{OSSIM_INSTALL_PREFIX}
    	PATH_SUFFIXES 
		lib
		include
)
SET(OSSIMVIDEO_NAMES ${OSSIMVIDEO_NAMES} ossim-video )
FIND_LIBRARY(OSSIMVIDEO_LIBRARY NAMES ${OSSIMVIDEO_NAMES}
	HINTS 
		$ENV{OSSIM_DEV_HOME}/ossim-video/lib
	PATHS
		$ENV{OSSIM_DEV_HOME}/ossim-video/lib
		$ENV{OSSIM_INSTALL_PREFIX}
    	PATH_SUFFIXES 
		lib
		Frameworks
)

# handle the QUIETLY and REQUIRED arguments and set OSSIMVIDEO_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OSSIMVIDEO  DEFAULT_MSG  OSSIMVIDEO_LIBRARY  OSSIMVIDEO_INCLUDE_DIR)

IF(OSSIMVIDEO_FOUND)
  SET( OSSIMVIDEO_LIBRARIES ${OSSIMVIDEO_LIBRARY} )
  SET( OSSIMVIDEO_INCLUDES ${OSSIMVIDEO_INCLUDE_DIR} )
ENDIF(OSSIMVIDEO_FOUND)

MARK_AS_ADVANCED(OSSIMVIDEO_INCLUDE_DIR OSSIMVIDEO_LIBRARY)

