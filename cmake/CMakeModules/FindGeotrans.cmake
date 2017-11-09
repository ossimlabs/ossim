#---
# File: FindGeotrans.cmake
#
# Find the native GeoTrans includes and libraries.
#
# This module defines:
#
# GEOTRANS_INCLUDE_DIR, where to find geotrans_config.h, etc.
# GEOTRANS_LIBRARY, libraries to link against to use Geotrans.
# GEOTRANS_FOUND, True if found, false if one of the above are not found.
# 
# For ossim, typically geos will be system installed which should be found; 
# or found in the ossim 3rd party dependencies directory from a geos build 
# and install.  If the latter it will rely on CMAKE_INCLUDE_PATH and 
# CMAKE_LIBRARY_PATH having the path to the party dependencies directory.
# 
# NOTE:
# 
# This script is specialized for ossim, e.g. looking in /usr/local/ossim.
#
# $Id$
#---

#---
# Find include path:
# Note: Version < 3.3.0 do not have geos.h in the geos sub directory; hence,
# the check for both "geos/geos.h" and "geos.h".
#---
find_path( GEOTRANS_INCLUDE_DIR geotrans/geotrans_config.h
           PATHS 
           /usr/include
           /usr/local/include
           /usr/local/ossim/include )

# Find Geotrans library:
find_library( Geotrans_LIB NAMES geotrans 
              PATHS 
              /usr/lib64 
              /usr/lib 
              /usr/local/lib
              /usr/local/ossim/lib )

# Set the Geotrans_LIBRARY:
if( Geotrans_LIB )
   set( GEOTRANS_LIBRARY ${Geotrans_LIB} CACHE STRING INTERNAL )
endif(Geotrans_LIB )

#---
# This function sets Geotrans_FOUND if variables are valid.
#--- 
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( GEOTRANS DEFAULT_MSG 
                                   GEOTRANS_LIBRARY 
                                   GEOTRANS_INCLUDE_DIR )

if( GEOTRANS_FOUND )
   if( NOT GEOTRANS_FIND_QUIETLY )
      message( STATUS "Found Geotrans..." )
   endif( NOT GEOTRANS_FIND_QUIETLY )
else( Geotrans_FOUND )
   if( NOT GEOTRANS_FIND_QUIETLY )
      message( WARNING "Could not find Geotrans" )
   endif( NOT GEOTRANS_FIND_QUIETLY )
endif( GEOTRANS_FOUND )

if( NOT GEOTRANS_FIND_QUIETLY )
   message( STATUS "GEOTRANS_INCLUDE_DIR=${GEOTRANS_INCLUDE_DIR}" )
   message( STATUS "GEOTRANS_LIBRARY=${GEOTRANS_LIBRARY}" )
endif( NOT GEOTRANS_FIND_QUIETLY )
