# - Find wms library
# Find the native wms includes and library
# This module defines
#  WMS_INCLUDE_DIR, where to find tiff.h, etc.
#  WMS_LIBRARIES, libraries to link against to use WMS.
#  WMS_FOUND, If false, do not try to use WMS.
# also defined, but not for general use are
#  WMS_LIBRARY, where to find the WMS library.
SET(CMAKE_FIND_FRAMEWORK "LAST")
FIND_PATH(WMS_INCLUDE_DIR wms/wms.h
	HINTS 
		$ENV{OSSIM_DEV_HOME}/libwms/include
		$ENV{OSSIM_DEV_HOME}/libwms/lib
		$ENV{OSSIM_INSTALL_PREFIX}/include
	PATHS
		$ENV{OSSIM_DEV_HOME}/libwms
		$ENV{OSSIM_INSTALL_PREFIX}
    	PATH_SUFFIXES 
		lib
		include

)

SET(WMS_NAMES ${WMS_NAMES} wms libwms)
FIND_LIBRARY(WMS_LIBRARY NAMES ${WMS_NAMES} 
	HINTS 
		$ENV{OSSIM_DEV_HOME}/liwms/lib
		$ENV{OSSIM_DEV_HOME}/libwms
		$ENV{OSSIM_INSTALL_PREFIX}
	PATHS
		$ENV{OSSIM_DEV_HOME}/libwms/lib
		$ENV{OSSIM_DEV_HOME}/libwms
		$ENV{OSSIM_INSTALL_PREFIX}
    	PATH_SUFFIXES 
		lib
		Frameworks
)

# handle the QUIETLY and REQUIRED arguments and set WMS_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(WMS  DEFAULT_MSG  WMS_LIBRARY  WMS_INCLUDE_DIR)

IF(WMS_FOUND)
  SET( WMS_LIBRARIES ${WMS_LIBRARY} )
ENDIF(WMS_FOUND)

MARK_AS_ADVANCED(WMS_INCLUDE_DIR WMS_LIBRARY)
