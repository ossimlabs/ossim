# - Find fftw3 library
# Find the native fftw3 includes and library
# This module defines
#  FFTW3_INCLUDE_DIR, where to find tiff.h, etc.
#  FFTW3_LIBRARIES, libraries to link against to use FFTW3.
#  FFTW3_FOUND, If false, do not try to use FFTW3.
# also defined, but not for general use are
#  FFTW3_LIBRARY, where to find the FFTW3 library.

FIND_PATH(FFTW3_INCLUDE_DIR fftw3.h)

SET(FFTW3_NAMES ${FFTW3_NAMES} fftw3 libfftw3)
FIND_LIBRARY(FFTW3_LIBRARY NAMES ${FFTW3_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set FFTW3_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FFTW3  DEFAULT_MSG  FFTW3_LIBRARY  FFTW3_INCLUDE_DIR)

IF(FFTW3_FOUND)
  SET( FFTW3_LIBRARIES ${FFTW3_LIBRARY} )
ENDIF(FFTW3_FOUND)

MARK_AS_ADVANCED(FFTW3_INCLUDE_DIR FFTW3_LIBRARY)
