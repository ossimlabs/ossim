# - Find ossimPlanet library
# Find the native ossimPlanet includes and library
# This module defines
#  OSSIMPLANET_INCLUDE_DIR, where to find tiff.h, etc.
#  OSSIMPLANET_LIBRARIES, libraries to link against to use OSSIM.
#  OSSIMPLANET_FOUND, If false, do not try to use OSSIM.
# also defined, but not for general use are
#  OSSIMPLANET_LIBRARY, where to find the OSSIM library.
SET(CMAKE_FIND_FRAMEWORK "LAST")
FIND_PATH(OSSIMPLANET_INCLUDE_DIR ossimPlanet/ossimPlanetExport.h ossimPlanetExport.h
	HINTS 
		$ENV{OSSIM_DEV_HOME}/ossimPlanet/include
		$ENV{OSSIM_DEV_HOME}/ossimPlanet/lib
		$ENV{OSSIM_INSTALL_PREFIX}/include
	PATHS
		$ENV{OSSIM_DEV_HOME}/ossimPlanet
		$ENV{OSSIM_INSTALL_PREFIX}
    	PATH_SUFFIXES 
		lib
		include
)

SET(OSSIMPLANET_NAMES ${OSSIMPLANET_NAMES} ossimPlanet )
FIND_LIBRARY(OSSIMPLANET_LIBRARY NAMES ${OSSIMPLANET_NAMES}
	HINTS 
		$ENV{OSSIM_DEV_HOME}/ossimPlanet/lib
	PATHS
		$ENV{OSSIM_DEV_HOME}/ossimPlanet/lib
		$ENV{OSSIM_INSTALL_PREFIX}
    	PATH_SUFFIXES 
		lib
		Frameworks
)

# handle the QUIETLY and REQUIRED arguments and set OSSIMPLANET_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OSSIMPLANET  DEFAULT_MSG  OSSIMPLANET_LIBRARY  OSSIMPLANET_INCLUDE_DIR)

IF(OSSIMPLANET_FOUND)
  SET( OSSIMPLANET_LIBRARIES ${OSSIMPLANET_LIBRARY} )
ENDIF(OSSIMPLANET_FOUND)

MARK_AS_ADVANCED(OSSIMPLANET_INCLUDE_DIR OSSIMPLANET_LIBRARY)
