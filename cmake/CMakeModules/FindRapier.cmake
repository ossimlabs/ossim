#---
# File: FindRapier.cmake
#
# Find the native rapier includes and library
#
# This module defines
#  RAPIER_INCLUDE_DIR, where to find tiff.h, etc.
#  RAPIER_LIBRARIES, libraries to link against to use RAPIER.
#  RAPIER_FOUND, If false, do not try to use RAPIER.
# also defined, but not for general use are
#  RAPIER_LIBRARY, where to find the RAPIER library.
# 
# $Id$
#---

find_path(RAPIER_INCLUDE_DIR rapier/common/rapierConstants.h
   PATHS
      $ENV{OSSIM_DEV_HOME}/Rapier
      ${CMAKE_INSTALL_PREFIX}
   PATH_SUFFIXES 
      include
)

find_library(RAPIER_LIBRARY
   NAMES rapier
   HINTS 
   PATHS
     $ENV{OSSIM_DEV_HOME}/Rapier
     $ENV{OSSIM_BUILD_DIR}
     ${CMAKE_INSTALL_PREFIX}
      /usr
      /usr/local
   PATH_SUFFIXES
      lib64
      lib )

#---
# This function sets RAPIER_FOUND if variables are valid.
#--- 
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(RAPIER  DEFAULT_MSG  RAPIER_LIBRARY  RAPIER_INCLUDE_DIR)

if(RAPIER_FOUND)
  set( RAPIER_LIBRARIES ${RAPIER_LIBRARY} )
endif(RAPIER_FOUND)
