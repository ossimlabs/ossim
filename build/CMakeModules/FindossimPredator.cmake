# - Find ossimPredator library
# Find the native ossimPredator includes and library
# This module defines
#  OSSIMPREDATOR_INCLUDE_DIR, where to find tiff.h, etc.
#  OSSIMPREDATOR_LIBRARIES, libraries to link against to use ossimPredator.
#  OSSIMPREDATOR_FOUND, If false, do not try to use ossimPredator.
# also defined, but not for general use are
#  OSSIMPREDATOR_LIBRARY, where to find the ossimPredator library.
SET(CMAKE_FIND_FRAMEWORK "LAST")
FIND_PATH(OSSIMPREDATOR_INCLUDE_DIR ossimPredator/ossimPredatorExport.h
	HINTS 
		$ENV{OSSIM_DEV_HOME}/ossimPredator/include
		$ENV{OSSIM_DEV_HOME}/ossimPredator/lib
		$ENV{OSSIM_INSTALL_PREFIX}/include
	PATHS
		$ENV{OSSIM_DEV_HOME}/ossim
		$ENV{OSSIM_INSTALL_PREFIX}
    	PATH_SUFFIXES 
		lib
		include
)
SET(OSSIMPREDATOR_NAMES ${OSSIMPREDATOR_NAMES} ossimPredator )
FIND_LIBRARY(OSSIMPREDATOR_LIBRARY NAMES ${OSSIMPREDATOR_NAMES}
	HINTS 
		$ENV{OSSIM_DEV_HOME}/ossimPredator/lib
	PATHS
		$ENV{OSSIM_DEV_HOME}/ossimPredator/lib
		$ENV{OSSIM_INSTALL_PREFIX}
    	PATH_SUFFIXES 
		lib
		Frameworks
)

# handle the QUIETLY and REQUIRED arguments and set OSSIMPREDATOR_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OSSIMPREDATOR  DEFAULT_MSG  OSSIMPREDATOR_LIBRARY  OSSIMPREDATOR_INCLUDE_DIR)

IF(OSSIMPREDATOR_FOUND)
  SET( OSSIMPREDATOR_LIBRARIES ${OSSIMPREDATOR_LIBRARY} )
  SET( OSSIMPREDATOR_INCLUDES ${OSSIMPREDATOR_INCLUDE_DIR} )
ENDIF(OSSIMPREDATOR_FOUND)

MARK_AS_ADVANCED(OSSIMPREDATOR_INCLUDE_DIR OSSIMPREDATOR_LIBRARY)
