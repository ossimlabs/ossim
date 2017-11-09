###
# -*- cmake -*-
#
# File:  FindOpenJPEG.cmake
#
# Original script was copied from:
# http://code.google.com/p/emeraldviewer/source/browse/indra/cmake
#
# $Id$
###

# - Find OpenJPEG
# Find the OpenJPEG includes and library
# This module defines
#  OPENJPEG_INCLUDE_DIR, where to find openjpeg.h, etc.
#  OPENJPEG_LIBRARIES, the libraries needed to use OpenJPEG.
#  OPENJPEG_FOUND, If false, do not try to use OpenJPEG.
# also defined, but not for general use are
#  OPENJPEG_LIBRARY, where to find the OpenJPEG library.

FIND_PATH(OPENJPEG_INCLUDE_DIR openjpeg.h
  PATHS
    ${CMAKE_INSTALL_PREFIX}/include
    /usr/local/include
    /usr/include
  PATH_SUFFIXES
    openjpeg
    openjpeg-1.5
    openjpeg-2.1
  DOC "Location of OpenJPEG Headers"
)
FIND_LIBRARY(MINIZIP_LIBRARY NAMES ${MINIZIP_NAMES} )
MARK_AS_ADVANCED( MINIZIP_LIBRARY MINIZIP_INCLUDE_DIR )

SET(OPENJPEG_NAMES ${OPENJPEG_NAMES} openjp2)
SET(OPENJPEG_NAMES ${OPENJPEG_NAMES} openjpeg)
FIND_LIBRARY(OPENJPEG_LIBRARY
  NAMES ${OPENJPEG_NAMES}
	PATHS
		${CMAKE_INSTALL_PREFIX}
		/usr
		/usr/local
		/usr/lib
	PATH_SUFFIXES
		lib64
		lib
		x86_64-linux-gnu )
  
IF (OPENJPEG_LIBRARY AND OPENJPEG_INCLUDE_DIR)
    SET(OPENJPEG_LIBRARIES ${OPENJPEG_LIBRARY})
    SET(OPENJPEG_FOUND "YES")
ELSE (OPENJPEG_LIBRARY AND OPENJPEG_INCLUDE_DIR)
  SET(OPENJPEG_FOUND "NO")
ENDIF (OPENJPEG_LIBRARY AND OPENJPEG_INCLUDE_DIR)

IF (OPENJPEG_FOUND)
   IF (NOT OPENJPEG_FIND_QUIETLY)
      MESSAGE(STATUS "Found OpenJPEG: ${OPENJPEG_LIBRARIES}")
   ENDIF (NOT OPENJPEG_FIND_QUIETLY)
ELSE (OPENJPEG_FOUND)
   IF (OPENJPEG_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find OpenJPEG library")
   ENDIF (OPENJPEG_FIND_REQUIRED)
ENDIF (OPENJPEG_FOUND)

MARK_AS_ADVANCED(OPENJPEG_LIBRARY OPENJPEG_INCLUDE_DIR)
