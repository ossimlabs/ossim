# - Find geotiff library
# Find the native geotiff includes and library
# This module defines
#  GEOTIFF_INCLUDE_DIR, where to find tiff.h, etc.
#  GEOTIFF_LIBRARIES, libraries to link against to use GEOTIFF.
#  GEOTIFF_FOUND, If false, do not try to use GEOTIFF.
# also defined, but not for general use are
#  GEOTIFF_LIBRARY, where to find the GEOTIFF library.

FIND_PATH(GEOTIFF_INCLUDE_DIR geotiff.h 
          PATHS 
          ${CMAKE_INSTALL_PREFIX}/include
          /usr/include/geotiff 
          /usr/include/libgeotiff 
          /usr/local/include/libgeotiff 
          /usr/local/include/geotiff)

SET(GEOTIFF_NAMES ${GEOTIFF_NAMES} geotiff_i geotiff libgeotiff_i libgeotiff)
FIND_LIBRARY(GEOTIFF_LIBRARY 
             NAMES ${GEOTIFF_NAMES}
             PATHS 
             ${CMAKE_INSTALL_PREFIX}/lib 
             /usr/local/lib 
             /usr/lib 
             /usr/lib/x86_64-linux-gnu)

# handle the QUIETLY and REQUIRED arguments and set GEOTIFF_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GEOTIFF  DEFAULT_MSG  GEOTIFF_LIBRARY  GEOTIFF_INCLUDE_DIR)

IF(GEOTIFF_FOUND)
  SET( GEOTIFF_LIBRARIES ${GEOTIFF_LIBRARY} )
ENDIF(GEOTIFF_FOUND)

MARK_AS_ADVANCED(GEOTIFF_INCLUDE_DIR GEOTIFF_LIBRARY)
