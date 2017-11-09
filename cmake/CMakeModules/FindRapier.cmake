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
   HINTS	
      $ENV{OSSIM_DEV_HOME}/Rapier/include
      $ENV{OSSIM_INSTALL_PREFIX}/include
   PATHS
      $ENV{OSSIM_DEV_HOME}/Rapier
      $ENV{OSSIM_INSTALL_PREFIX}
   PATH_SUFFIXES 
      include
)

find_library(RAPIER_LIBRARY
   NAMES rapier
   HINTS 
      $ENV{OSSIM_DEV_HOME}/Rapier/lib
      $ENV{OSSIM_DEV_HOME}/build/lib
   PATHS
      $ENV{OSSIM_DEV_HOME}/Rapier/lib
      $ENV{OSSIM_INSTALL_PREFIX}
   PATH_SUFFIXES 
      lib
)

#---
# This function sets RAPIER_FOUND if variables are valid.
#--- 
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(RAPIER  DEFAULT_MSG  RAPIER_LIBRARY  RAPIER_INCLUDE_DIR)

if(RAPIER_FOUND)
  set( RAPIER_LIBRARIES ${RAPIER_LIBRARY} )
endif(RAPIER_FOUND)
