#---
# $Id$
#
# Find the stlport includes and library
# 
# This module defines:
#  STLPORT_INCLUDE_DIR, where to find iostream, string, etc.
#  STLPORT_LIBRARIES, libraries to link against to use StlPort.
#  STLPORT_FOUND, If false, do not try to use STLPORT.
#---

FIND_PATH(STLPORT_INCLUDE_DIR iostream PATHS C:/STLport-5.2.1/stlport D:/STLport-5.2.1/stlport)

SET(STLPORT_NAMES ${STLPORT_NAMES} stlport5.2.lib)
FIND_LIBRARY(STLPORT_LIBRARY NAMES ${STLPORT_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set STLPORT_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(STLPORT  DEFAULT_MSG  STLPORT_LIBRARY  STLPORT_INCLUDE_DIR)

IF(STLPORT_FOUND)
  SET( STLPORT_LIBRARIES ${STLPORT_LIBRARY} )
ENDIF(STLPORT_FOUND)

MARK_AS_ADVANCED(STLPORT_INCLUDE_DIR STLPORT_LIBRARY)
