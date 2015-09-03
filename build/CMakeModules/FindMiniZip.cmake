# - Find minizip
# Find the native MINIZIP includes and library
#
#  MINIZIP_INCLUDE_DIRS - where to find minizip's zip.h, etc.
#  MINIZIP_LIBRARIES    - List of libraries when using minizip.
#  MINIZIP_FOUND        - True if minizip found.

IF (MINIZIP_INCLUDE_DIR)
  # Already in cache, be silent
  SET(MINIZIP_FIND_QUIETLY TRUE)
ENDIF (MINIZIP_INCLUDE_DIR)

FIND_PATH(MINIZIP_INCLUDE_DIR minizip/zip.h)

SET(MINIZIP_NAMES minizip )
FIND_LIBRARY(MINIZIP_LIBRARY NAMES ${MINIZIP_NAMES} )
MARK_AS_ADVANCED( MINIZIP_LIBRARY MINIZIP_INCLUDE_DIR )

# Per-recommendation
SET(MINIZIP_INCLUDE_DIRS "${MINIZIP_INCLUDE_DIR}")
SET(MINIZIP_LIBRARIES    "${MINIZIP_LIBRARY}")

# handle the QUIETLY and REQUIRED arguments and set MINIZIP_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MINIZIP DEFAULT_MSG MINIZIP_LIBRARIES MINIZIP_INCLUDE_DIRS)
