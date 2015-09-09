# - Find csmApi library
# Find the native csmApi includes and library
# This module defines
#  CSMAPI_INCLUDE_DIRS, where to find cam api headers, etc.
#  CSMAPI_LIBRARIES, libraries to link against to use csmApi.
#  CSMAPI_FOUND, If false, do not try to use csmApi.
# also defined, but not for general use are
#  CSMAPI_LIBRARY, where to find the CSMAPI library.
SET(CMAKE_FIND_FRAMEWORK "LAST")
FIND_PATH(CSMAPI_INCLUDE_DIRS SMManager.h
	HINTS 
		$ENV{OSSIM_INSTALL_PREFIX}/include/csmApi
		$ENV{OSSIM_DEV_HOME}/csmApi/include
		$ENV{OSSIM_DEV_HOME}/csmApi/include/TSM
		$ENV{OSSIM_DEV_HOME}/csmApi/lib
	PATHS
		$ENV{OSSIM_INSTALL_PREFIX}/include/csmApi
		$ENV{OSSIM_DEV_HOME}/csmApi/include
		$ENV{OSSIM_DEV_HOME}/csmApi/include/TSM
    	PATH_SUFFIXES 
		lib
		include
)

set(CSMAPI_NAMES ${CSMAPI_NAMES} csmApi libcsmApi)
find_library(CSMAPI_LIBRARY NAMES ${CSMAPI_NAMES}
   PATHS
      $ENV{OSSIM_INSTALL_PREFIX}/lib${LIBSUFFIX}
      $ENV{OSSIM_BUILD_DIR}/build_ossim/lib${LIBSUFFIX}
      $ENV{OSSIM_BUILD_DIR}/lib${LIBSUFFIX}
      $ENV{OSSIM_DEV_HOME}/build/build_ossim/lib${LIBSUFFIX}
      $ENV{OSSIM_DEV_HOME}/build/lib${LIBSUFFIX}
      $ENV{OSSIM_DEV_HOME}/ossim/lib${LIBSUFFIX}
      $ENV{OSSIM_INSTALL_PREFIX}
   PATH_SUFFIXES
      lib
      Frameworks
)

#---
# This function sets CSMAPI_FOUND if variables are valid.
#--- 
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( CSMAPI DEFAULT_MSG 
                                   CSMAPI_LIBRARY 
                                   CSMAPI_INCLUDE_DIR )

if(CSMAPI_FOUND)
   set( CSMAPI_LIBRARIES ${CSMAPI_LIBRARY} )
   set( CSMAPI_INCLUDES  ${CSMAPI_INCLUDE_DIR} ${CSMAPI_INCLUDE_DIR}/TSM )
else( CSMAPI_FOUND )
   if( NOT CSMAPI_FIND_QUIETLY )
      message( WARNING "Could not find CSMAPI" )
   endif( NOT CSMAPI_FIND_QUIETLY )
endif(CSMAPI_FOUND)

if( NOT CSMAPI_FIND_QUIETLY )
   message( STATUS "CSMAPI_INCLUDE_DIR=${CSMAPI_INCLUDE_DIR}" )
   message( STATUS "CSMAPI_LIBRARY=${CSMAPI_LIBRARY}" )
endif( NOT CSMAPI_FIND_QUIETLY )

mark_as_advanced(CSMAPI_INCLUDES CSMAPI_INCLUDE_DIR CSMAPI_LIBRARY)
