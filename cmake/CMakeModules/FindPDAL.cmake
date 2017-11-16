#---
# File: FindPDAL.cmake
#
# Find the native Point Data Abstraction Library(PDAL) includes and libraries.
#
# This module defines:
#
# PDAL_INCLUDE_DIR, where to find pdal.h, etc.
# PDAL_LIBRARIES, libraries to link against to use PDAL and rialto.
# PDAL_FOUND, True if found, false if one of the above are not found.
# 
# NOTE: 
# This script is specialized for ossim, e.g. looking in /usr/local/ossim.
#
# $Id$
#---
#---
# Find include path:  "pdal_defines.h" installs to install "prefix" with pdal 
# includes under "pdal" sub directory.
#---
find_path( PDAL_INCLUDE_DIR pdal/pdal.hpp)

find_library(PDAL_CPP_LIBRARY NAMES pdalcpp)
find_library(PDAL_UTIL_LIBRARY NAMES pdal_util)
find_library(RIALTO_LIBRARY NAMES rialto)
find_library(LASZIP_LIBRARY NAMES laszip)

message( STATUS "PDAL_INCLUDE_DIR             = ${PDAL_INCLUDE_DIR}" )
message( STATUS "PDAL_CPP_LIBRARY             = ${PDAL_CPP_LIBRARY}" )
message( STATUS "PDAL_UTIL_LIBRARY            = ${PDAL_UTIL_LIBRARY}" )
message( STATUS "RIALTO_LIBRARY               = ${RIALTO_LIBRARY}" )
message( STATUS "LASZIP_LIBRARY               = ${LASZIP_LIBRARY}" )

set(PDAL_FOUND "NO")

if (PDAL_INCLUDE_DIR AND PDAL_CPP_LIBRARY AND PDAL_UTIL_LIBRARY AND RIALTO_LIBRARY AND LASZIP_LIBRARY)

   set(PDAL_FOUND "YES")
   set(PDAL_LIBRARIES ${PDAL_CPP_LIBRARY} ${PDAL_UTIL_LIBRARY} ${RIALTO_LIBRARY} ${LASZIP_LIBRARY})
   message( STATUS "PDAL_LIBRARIES = ${PDAL_LIBRARIES}" )

else(PDAL_INCLUDE_DIR AND PDAL_CPP_LIBRARY AND PDAL_UTIL_LIBRARY AND RIALTO_LIBRARY AND LASZIP_LIBRARY)

   message( WARNING "Could not find PDAL" )

endif(PDAL_INCLUDE_DIR AND PDAL_CPP_LIBRARY AND PDAL_UTIL_LIBRARY AND RIALTO_LIBRARY AND LASZIP_LIBRARY)


