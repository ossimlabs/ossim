# - Find JPEG12
#
# ---
# This is a specialized find for custom built jpeg library with 12 samples
# enabled.
# Specifically looks for library: /usr/lib64/libjpeg12.so
# and header file:  /usr/include/jpeg12/jpeglib.h
# ---
#
#
# Find the native JPEG includes and library
# This module defines
#  JPEG12_INCLUDE_DIR, where to find jpeglib.h, etc.
#  JPEG12_LIBRARIES, the libraries needed to use JPEG.
#  JPEG12_FOUND, If false, do not try to use JPEG.
# also defined, but not for general use are
#  JPEG12_LIBRARY, where to find the JPEG library.

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

find_path( JPEG12_INCLUDE_DIR jpeg12/jpeglib.h
           PATHS
           ${CMAKE_INSTALL_PREFIX}/include
           /usr/include
           /usr/local/include )

set( JPEG12_NAMES ${JPEG12_NAMES} jpeg12 libjpeg12 )

find_library( JPEG12_LIBRARY
              NAMES ${JPEG12_NAMES}
              PATHS
              /usr/lib64
              /usr/local/lib64
              /usr/lib
              /usr/local/lib )

#---
# This function sets JPEG12_FOUND if variables are valid.
#--- 
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JPEG12 DEFAULT_MSG JPEG12_LIBRARY JPEG12_INCLUDE_DIR)

if( JPEG12_FOUND )
   if( NOT JPEG12_FIND_QUIETLY )
      message( STATUS "Found JPEG12..." )
   endif( NOT JPEG12_FIND_QUIETLY )
else( JPEG12_FOUND )
   if( NOT JPEG12_FIND_QUIETLY )
      message( WARNING "Could not find JPEG12" )
   endif( NOT JPEG12_FIND_QUIETLY )
endif( JPEG12_FOUND )

if( NOT JPEG12_FIND_QUIETLY )
   message( STATUS "JPEG12_INCLUDE_DIR=${JPEG12_INCLUDE_DIR}" )
   message( STATUS "JPEG12_LIBRARY=${JPEG12_LIBRARY}" )
endif( NOT JPEG12_FIND_QUIETLY )
