#---
# File: FindLibuuid.cmake
#
# Find the native Libuuid includes and libraries.
#
# This module defines:
#
# LIBUUID_INCLUDE_DIR, where to find header files, e.g. uuid.h, etc.
# LIBUUID_LIBRARY, libraries to link against to use libuuid.
# LIBUUID_FOUND, True if found, false if one of the above are not found.
# 
# $Id$
#---

#---
# Find include path:
#---
find_path( LIBUUID_INCLUDE_DIR uuid.h
           PATHS
           ${CMAKE_INSTALL_PREFIX}/include
           /usr/include/uuid
	   /usr/include
	   /usr/local/include/uuid
	   /usr/local/include
	   /opt/local/include/uuid
	   /opt/local/include )

# Find Libuuid library:
find_library( LIBUUID_LIBRARY NAMES uuid
              PATHS
              ${CMAKE_INSTALL_PREFIX}/lib64
              ${CMAKE_INSTALL_PREFIX}/lib
              /usr/lib64 
              /usr/lib
	      /usr/local/lib64
	      /usr/local/lib
	      /opt/local/lib )
	    
#---
# This function sets Libuuid_FOUND if variables are valid.
#--- 
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( Libuuid DEFAULT_MSG 
                                   LIBUUID_LIBRARY
                                   LIBUUID_INCLUDE_DIR )

if( LIBUUID_FOUND )
   if( NOT LIBUUID_FIND_QUIETLY )
      message( STATUS "Found LIBUUID..." )
   endif( NOT LIBUUID_FIND_QUIETLY )
else()
   if( NOT LIBUUID_FIND_QUIETLY )
      message( WARNING "Could not find LIBUUID" )
   endif( NOT LIBUUID_FIND_QUIETLY )
endif()

if( NOT LIBUUID_FIND_QUIETLY )
   message( STATUS "LIBUUID_INCLUDE_DIR=${LIBUUID_INCLUDE_DIR}" )
   message( STATUS "LIBUUID_LIBRARY=${LIBUUID_LIBRARY}" )
endif( NOT LIBUUID_FIND_QUIETLY )

MARK_AS_ADVANCED(LIBUUID_INCLUDE_DIR LIBUUID_LIBRARY)
