#---
# File: FindShapelib.cmake
#
# Find the native Shapelib includes and libraries.
#
# This module defines:
#
# SHAPELIB_INCLUDE_DIR, where to find header files, e.g. fcgio.h, etc.
# SHAPELIB_LIBRARY, libraries to link against to use Shapelib.
# SHAPELIB_FOUND, True if found, false if one of the above are not found.
# 
# $Id$
#---

#---
# Find include path:
#---
find_path( SHAPELIB_INCLUDE_DIR shapefil.h
           PATHS
           ${CMAKE_INSTALL_PREFIX}/include
           /usr/include
           /usr/local/include )

# Find Shapelib library:
find_library( SHAPELIB_LIBRARY NAMES shp
              PATHS
              ${CMAKE_INSTALL_PREFIX}/lib64
              ${CMAKE_INSTALL_PREFIX}/lib
              /usr/lib64 
              /usr/lib 
              /usr/local/lib )
	    
#---
# This function sets Shapelib_FOUND if variables are valid.
#--- 
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( Shapelib DEFAULT_MSG 
                                   SHAPELIB_LIBRARY
                                   SHAPELIB_INCLUDE_DIR )

if( SHAPELIB_FOUND )
   if( NOT SHAPELIB_FIND_QUIETLY )
      message( STATUS "Found SHAPELIB..." )
   endif( NOT SHAPELIB_FIND_QUIETLY )
else()
   if( NOT SHAPELIB_FIND_QUIETLY )
      message( WARNING "Could not find SHAPELIB" )
   endif( NOT SHAPELIB_FIND_QUIETLY )
endif()

if( NOT SHAPELIB_FIND_QUIETLY )
   message( STATUS "SHAPELIB_INCLUDE_DIR=${SHAPELIB_INCLUDE_DIR}" )
   message( STATUS "SHAPELIB_LIBRARY=${SHAPELIB_LIBRARY}" )
endif( NOT SHAPELIB_FIND_QUIETLY )

MARK_AS_ADVANCED(SHAPELIB_INCLUDE_DIR SHAPELIB_LIBRARY)
