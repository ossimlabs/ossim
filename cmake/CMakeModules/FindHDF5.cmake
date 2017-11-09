#---
# File: FindHDF5.cmake
#
# Find the native HDF5 includes and libraries.
#
# This module defines:
#
# HDF5_INCLUDE_DIR, where to find geos.h, etc.
# HDF5_LIBRARIES, libraries to link against to use HDF5.
# HDF5_FOUND, True if found, false if one of the above are not found.
# 
# NOTE: 
#
# This script is specialized for ossim. HDF5 rpm created to fix conflict with
# system installed hdf5 packages that do NOT have compression(szip) support.
#
# $Id$
#---

# Find include path:
find_path( HDF5_INCLUDE_DIR hdf5.h
           PATHS 
           $ENV{HDF5_DIR}/include
           /usr/include
           /usr/local/include
           /usr/local/ossim/include )

# Find HDF5 library:
find_library( HDF5_LIB NAMES hdf5
              PATHS
              $ENV{HDF5_DIR}/lib64
              $ENV{HDF5_DIR}/lib
              /usr/lib64
              /usr/lib
              /usr/local/lib
              /usr/local/ossim/lib )

# Find HDF5 CPP library:
find_library( HDF5_CPP_LIB NAMES hdf5_cpp
              PATHS
              $ENV{HDF5_DIR}/lib64
              $ENV{HDF5_DIR}/lib
              /usr/lib64
              /usr/lib
              /usr/local/lib
              /usr/local/ossim/lib )

# Set the HDF5_LIBRARIES:
if( HDF5_LIB AND HDF5_CPP_LIB )
   set( HDF5_LIBRARIES ${HDF5_LIB} ${HDF5_CPP_LIB} CACHE STRING INTERNAL )
endif(HDF5_LIB AND HDF5_CPP_LIB )

#---
# This function sets HDF5_FOUND if variables are valid.
#--- 
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( HDF5 DEFAULT_MSG 
                                   HDF5_LIBRARIES 
                                   HDF5_INCLUDE_DIR )

if( HDF5_FOUND )
   if( NOT HDF5_FIND_QUIETLY )
      message( STATUS "Found HDF5..." )
   endif( NOT HDF5_FIND_QUIETLY )
else( HDF5_FOUND )
   if( NOT HDF5_FIND_QUIETLY )
      message( WARNING "Could not find HDF5" )
   endif( NOT HDF5_FIND_QUIETLY )
endif( HDF5_FOUND )

if( NOT HDF5_FIND_QUIETLY )
   message( STATUS "HDF5_INCLUDE_DIR=${HDF5_INCLUDE_DIR}" )
   message( STATUS "HDF5_LIBRARIES=${HDF5_LIBRARIES}" )
endif( NOT HDF5_FIND_QUIETLY )

