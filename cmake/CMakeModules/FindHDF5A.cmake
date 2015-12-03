#---
# File: FindHDF5A.cmake
#
# Find the native HDF5A includes and libraries.
#
# This module defines:
#
# HDF5A_INCLUDE_DIR, where to find geos.h, etc.
# HDF5A_LIBRARIES, libraries to link against to use HDF5A.
# HDF5A_FOUND, True if found, false if one of the above are not found.
# 
# NOTE: 
#
# This script is specialized for ossim. HDF5A rpm created to fix conflict with
# system installed hdf5 packages that do NOT have compression(szip) support.
#
# $Id$
#---

# Find include path:
find_path( HDF5A_INCLUDE_DIR hdf5.h
           PATHS 
           ${CMAKE_INSTALL_PREFIX}/include
           /usr/include
           /usr/local/include
           /usr/local/ossim/include )

# Find HDF5A library:
find_library( HDF5A_LIB NAMES hdf5a hdf5
              PATHS
              ${CMAKE_INSTALL_PREFIX}/lib64
              /usr/lib64
              ${CMAKE_INSTALL_PREFIX}/lib
              /usr/lib
              /usr/local/lib
              /usr/local/ossim/lib )

# Find HDF5A CPP library:
find_library( HDF5A_CPP_LIB NAMES hdf5a_cpp hdf5_cpp
              PATHS
              ${CMAKE_INSTALL_PREFIX}/lib64
              /usr/lib64
              ${CMAKE_INSTALL_PREFIX}/lib
              /usr/lib
              /usr/local/lib
              /usr/local/ossim/lib )

# Set the HDF5A_LIBRARIES:
if( HDF5A_LIB AND HDF5A_CPP_LIB )
   set( HDF5A_LIBRARIES ${HDF5A_LIB} ${HDF5A_CPP_LIB} CACHE STRING INTERNAL )
endif(HDF5A_LIB AND HDF5A_CPP_LIB )

#---
# This function sets HDF5A_FOUND if variables are valid.
#--- 
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( HDF5A DEFAULT_MSG 
                                   HDF5A_LIBRARIES 
                                   HDF5A_INCLUDE_DIR )

if( HDF5A_FOUND )
   if( NOT HDF5A_FIND_QUIETLY )
      message( STATUS "Found HDF5A..." )
   endif( NOT HDF5A_FIND_QUIETLY )
else( HDF5A_FOUND )
   if( NOT HDF5A_FIND_QUIETLY )
      message( WARNING "Could not find HDF5A" )
   endif( NOT HDF5A_FIND_QUIETLY )
endif( HDF5A_FOUND )

if( NOT HDF5A_FIND_QUIETLY )
   message( STATUS "HDF5A_INCLUDE_DIR=${HDF5A_INCLUDE_DIR}" )
   message( STATUS "HDF5A_LIBRARIES=${HDF5A_LIBRARIES}" )
endif( NOT HDF5A_FIND_QUIETLY )
