# - Find ossim library
# Find the native ossim includes and library
# This module defines
#  OMS_INCLUDE_DIR, where to find tiff.h, etc.
#  OMS_LIBRARIES, libraries to link against to use OSSIM.
#  OMS_FOUND, If false, do not try to use OSSIM.
# also defined, but not for general use are
#  OMS_LIBRARY, where to find the OSSIM library.

SET(CMAKE_FIND_FRAMEWORK "LAST")
FIND_PATH(OMS_INCLUDE_DIR oms/ossimVersion.h
	HINTS 
		$ENV{OSSIM_DEV_HOME}/oms/coms/include
		$ENV{OSSIM_DEV_HOME}/oms/lib
		$ENV{OSSIM_INSTALL_PREFIX}/include
	PATHS
		$ENV{OSSIM_DEV_HOME}/oms/coms
		$ENV{OSSIM_INSTALL_PREFIX}
    	PATH_SUFFIXES 
		lib
		include
)
SET(OSSIM_NAMES ${OSSIM_NAMES} oms )
FIND_LIBRARY(OSSIM_LIBRARY NAMES ${OSSIM_NAMES}
	HINTS 
		$ENV{OSSIM_DEV_HOME}/ossim/lib
	PATHS
		$ENV{OSSIM_DEV_HOME}/ossim/lib
		$ENV{OSSIM_INSTALL_PREFIX}
    	PATH_SUFFIXES 
		lib
		Frameworks
)

# handle the QUIETLY and REQUIRED arguments and set OSSIM_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OMS  DEFAULT_MSG  OMS_LIBRARY  OMS_INCLUDE_DIR)

IF(OMS_FOUND)
  SET( OMS_LIBRARIES ${OMS_LIBRARY} )
ENDIF(OMS_FOUND)

MARK_AS_ADVANCED(OMS_INCLUDE_DIR OMS_LIBRARY)
