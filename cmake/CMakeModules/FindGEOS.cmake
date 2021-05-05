#---
# File: FindGEOS.cmake
#
# Find the native GEOS(Geometry Engine - Open Source) includes and libraries.
#
# This module defines:
#
# GEOS_INCLUDE_DIR, where to find geos.h, etc.
# GEOS_C_LIBRARY
# GEOS_CPP_LIBRARY
# GEOS_LIBRARIES, C and CPP libraries to link against to use GEOS.  Currently
# there are two looked for, geos and geos_c libraries.
# GEOS_FOUND, True if found, false if one of the above are not found.
# 
# For ossim, typically geos will be system installed which should be found; 
# or found in the ossim 3rd party dependencies directory from a geos build 
# and install.  If the latter it will rely on CMAKE_INCLUDE_PATH and 
# CMAKE_LIBRARY_PATH having the path to the party dependencies directory.
# 
# NOTE: 
# This script is specialized for ossim, e.g. looking in /usr/local/ossim.
#
# $Id$
#---

#---
# Find include path:
# Note: Ubuntu 14.04+ did not have geos.h (not included in any ossim src). 
# Instead looking for Geometry.h
#---

find_path( GEOS_INCLUDE_DIR geos_c.h
           PATHS 
           $ENV{GEOS_DIR}/include
           ${GEOS_DIR}/include
           /usr/local/include)

# Find GEOS library:
find_library( GEOS_CPP_LIBRARY NAMES geos
              PATHS
              $ENV{GEOS_DIR}/lib
              ${GEOS_DIR}/lib
              /usr/local/lib
              /usr/local/lib64)

# Find GEOS C library:
find_library( GEOS_C_LIBRARY NAMES geos_c 
              PATHS 
              $ENV{GEOS_DIR}/lib
              ${GEOS_DIR}/lib)

# Set the GEOS_LIBRARY:
if( GEOS_CPP_LIBRARY AND GEOS_C_LIBRARY )
   set( GEOS_LIBRARIES ${GEOS_CPP_LIBRARY} ${GEOS_C_LIBRARY} CACHE STRING INTERNAL )
endif( GEOS_CPP_LIBRARY AND GEOS_C_LIBRARY )

#---
# This function sets GEOS_FOUND if variables are valid.
#--- 
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( GEOS DEFAULT_MSG
                                   GEOS_LIBRARIES 
                                   GEOS_INCLUDE_DIR )

if( GEOS_FOUND )
   if( NOT GEOS_FIND_QUIETLY )
      message( STATUS "Found GEOS..." )
   endif( NOT GEOS_FIND_QUIETLY )
else( GEOS_FOUND )
   if( NOT GEOS_FIND_QUIETLY )
      message( WARNING "Could not find GEOS" )
   endif( NOT GEOS_FIND_QUIETLY )
endif( GEOS_FOUND )

if( NOT GEOS_FIND_QUIETLY )
   message( STATUS "GEOS_INCLUDE_DIR=${GEOS_INCLUDE_DIR}" )
   message( STATUS "GEOS_C_LIBRARY=${GEOS_C_LIBRARY}" )
   message( STATUS "GEOS_CPP_LIBRARY=${GEOS_CPP_LIBRARY}" )     
   message( STATUS "GEOS_LIBRARIES=${GEOS_LIBRARIES}" )
endif( NOT GEOS_FIND_QUIETLY )
