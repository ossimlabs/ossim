# - Find wms library
# Find the native wms includes and library
# This module defines
#  OSSIMWMS_INCLUDE_DIR, where to find tiff.h, etc.
#  OSSIMWMS_LIBRARIES, libraries to link against to use WMS.
#  OSSIMWMS_FOUND, If false, do not try to use WMS.
# also defined, but not for general use are
#  WMS_LIBRARY, where to find the WMS library.
SET(CMAKE_FIND_FRAMEWORK "LAST")
FIND_PATH(OSSIMWMS_INCLUDE_DIR wms/wms.h
	HINTS 
		$ENV{OSSIM_DEV_HOME}/ossim-wms/include
		$ENV{OSSIM_DEV_HOME}/ossim-wms/lib
		$ENV{OSSIM_INSTALL_PREFIX}/include
	PATHS
		$ENV{OSSIM_DEV_HOME}/ossim-wms
		$ENV{OSSIM_INSTALL_PREFIX}
    	PATH_SUFFIXES 
		lib
		include

)

SET(OSSIMWMS_NAMES ${OSSIMWMS_NAMES} wms ossim-wms)
FIND_LIBRARY(OSSIMWMS_LIBRARY NAMES ${OSSIMWMS_NAMES} 
	HINTS 
		$ENV{OSSIM_DEV_HOME}/ossim-wms/lib
		$ENV{OSSIM_DEV_HOME}/ossim-wms
		$ENV{OSSIM_INSTALL_PREFIX}
	PATHS
		$ENV{OSSIM_DEV_HOME}/ossim-wms/lib
		$ENV{OSSIM_DEV_HOME}/ossim-wms
		$ENV{OSSIM_INSTALL_PREFIX}
    	PATH_SUFFIXES 
		lib
		Frameworks
)

# handle the QUIETLY and REQUIRED arguments and set OSSIMWMS_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OSSIMWMS  DEFAULT_MSG  OSSIMWMS_LIBRARY  OSSIMWMS_INCLUDE_DIR)

IF(OSSIMWMS_FOUND)
  SET( OSSIMWMS_LIBRARIES ${OSSIMWMS_LIBRARY} )
ENDIF(OSSIMWMS_FOUND)

MARK_AS_ADVANCED(OSSIMWMS_INCLUDE_DIR OSSIMWMS_LIBRARY)
