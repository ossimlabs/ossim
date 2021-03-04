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
          $ENV{GEOTIFF_DIR}/include
          /usr/include/geotiff 
          /usr/include/libgeotiff 
          /usr/local/include/libgeotiff 
          /usr/local/include/geotiff)

SET(GEOTIFF_RELEASE_NAMES ${GEOTIFF_NAMES} geotiff_i geotiff libgeotiff_i libgeotiff)
SET(GEOTIFF_DEBUG_NAMES geotiff_d_i geotiff_d libgeotiff_d_i libgeotiff_d)
FIND_LIBRARY(GEOTIFF_LIBRARY_RELEASE 
             NAMES ${GEOTIFF_RELEASE_NAMES}
             PATHS 
             $ENV{GEOTIFF_DIR}/lib
             $ENV{GEOTIFF_DIR}/lib64)
FIND_LIBRARY(GEOTIFF_LIBRARY_DEBUG
             NAMES ${GEOTIFF_DEBUG_NAMES}
             PATHS 
             $ENV{GEOTIFF_DIR}/debug/lib
             $ENV{GEOTIFF_DIR}/debug/lib64)

INCLUDE(SelectLibraryConfigurations)
select_library_configurations(GEOTIFF)
set(GEOTIFF_LIBRARIES ${GEOTIFF_LIBRARY})

# handle the QUIETLY and REQUIRED arguments and set GEOTIFF_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GEOTIFF  DEFAULT_MSG  GEOTIFF_LIBRARY  GEOTIFF_INCLUDE_DIR)

MARK_AS_ADVANCED(GEOTIFF_INCLUDE_DIR GEOTIFF_LIBRARY)
