# - Find wms library
# Find the native wms includes and library
# This module defines
#  OSSIMWMS_INCLUDE_DIR, where to find tiff.h, etc.
#  OSSIMWMS_LIBRARIES, libraries to link against to use WMS.
#  OSSIMWMS_FOUND, If false, do not try to use WMS.
# also defined, but not for general use are
#  WMS_LIBRARY, where to find the WMS library.
IF(TARGET ossim-wms)
  IF(DEFINED OSSIM_DEV_HOME)
    SET(OSSIMWMS_INCLUDE_DIR "${OSSIM_DEV_HOME}/ossim-wms/include")
  ELSEIF(DEFINED ENV{OSSIM_DEV_HOME})
    SET(OSSIMWMS_INCLUDE_DIR "$ENV{OSSIM_DEV_HOME}/ossim-wms/include")
  ENDIF()
  SET(OSSIMWMS_LIBRARY ossim-wms)
ELSE()
  SET(CMAKE_FIND_FRAMEWORK "LAST")
  FIND_PATH(OSSIMWMS_INCLUDE_DIR wms/wms.h
    PATHS
    $ENV{OSSIM_DEV_HOME}/ossim-wms/include
    $ENV{OSSIM_BUILD_DIR}/../ossim-wms/include
    $ENV{OSSIM_INSTALL_PREFIX}/include
    PATH_SUFFIXES include)

  SET(OSSIMWMS_NAMES ${OSSIMWMS_NAMES} ossim-wms wms libossim-wms libwms)
  FIND_LIBRARY(OSSIMWMS_LIBRARY NAMES ${OSSIMWMS_NAMES}
    PATHS
    $ENV{OSSIM_BUILD_DIR}/lib64
    $ENV{OSSIM_BUILD_DIR}/lib
    $ENV{OSSIM_INSTALL_PREFIX}/lib64
    $ENV{OSSIM_INSTALL_PREFIX}/lib)
ENDIF()

# handle the QUIETLY and REQUIRED arguments and set OSSIMWMS_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OSSIMWMS  DEFAULT_MSG  OSSIMWMS_LIBRARY  OSSIMWMS_INCLUDE_DIR)

IF(OSSIMWMS_FOUND)
  SET( OSSIMWMS_LIBRARIES ${OSSIMWMS_LIBRARY} )
ENDIF(OSSIMWMS_FOUND)

MARK_AS_ADVANCED(OSSIMWMS_INCLUDE_DIR OSSIMWMS_LIBRARY)
