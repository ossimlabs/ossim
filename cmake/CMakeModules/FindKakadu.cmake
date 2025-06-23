#---
# File: FindKakadu.cmake
#
# Find the Kakadu includes and libraries.
#
# This module defines:
#
# KAKADU_INCLUDE_DIR (overridable)
# KAKADU_LIBRARY     (overridable)
# KAKADU_AUX_LIBRARY (overridable)
# KAKADU_LIBRARIES, libraries to link against to use Kakadu.
# KAKADU_FOUND, True if found, false if one of the above are not found.
#
# NOTES: 
# This script is specialized for ossim.
# Library search includes speed pack form(vs, as) and
# non speed pack(v, a) forms.
# e.g.:
# libkdu_as84R.so  libkdu_vs84R.so (linux
# libkdu_a84R.so  libkdu_v84R.so 
# libkdu.a libkdu_aux.a
# Windows: kdu_a81R.so  kdu_v81R.so
# Windows speedpack: kdu_as81R.so  kdu_vs81R.so
#
# Current version searches hard coded for 84 and 83.
#
# $Id$
#---

#---
# Find include path:
#---
find_path( KAKADU_INCLUDE_DIR kdu_compressed.h
           PATHS 
           ${CMAKE_INSTALL_PREFIX}/include/kakadu
	   ${CMAKE_INSTALL_PREFIX}../include/kakadu
	   ${CMAKE_INSTALL_PREFIX}/include/kakadu/managed/all_includes
           /usr/local/ossim/include/kakadu
	   /usr/local/ossim/include/kakadu/managed/all_includes
	   /usr/local/include/kakadu )

# Find Kakadu library:
find_library(
  KAKADU_LIB
  NAMES kdu_vs84R kdu_v84R kdu_vs83R kdu_v83R kdu
  PATHS
  ${CMAKE_INSTALL_PREFIX}/lib64
  ${CMAKE_INSTALL_PREFIX}/lib
  /usr/local/ossim/lib64
  /usr/local/ossim/lib
  /usr/local/lib64
  /usr/local/lib
  /usr/lib64 
  /usr/lib )

set( KAKADU_LIBRARY ${KAKADU_LIB} CACHE STRING INTERNAL )	    
	    
# Find Kakadu Auxilary library:
find_library(
  KAKADU_AUX_LIB
  NAMES kdu_as84R kdu_a84R kdu_as83R kdu_a83R kdu_aux
  PATHS 
  ${CMAKE_INSTALL_PREFIX}/lib64
  ${CMAKE_INSTALL_PREFIX}/lib
  /usr/local/ossim/lib64
  /usr/local/ossim/lib
  /usr/local/lib64
  /usr/local/lib
  /usr/lib64 
  /usr/lib )

set( KAKADU_AUX_LIBRARY ${KAKADU_AUX_LIB} CACHE STRING INTERNAL )	    


# Set the KAKADU_LIBRARIES:
if( KAKADU_LIBRARY AND KAKADU_AUX_LIBRARY )
   set( KAKADU_LIBRARIES ${KAKADU_LIBRARY} ${KAKADU_AUX_LIBRARY} CACHE STRING INTERNAL )
endif()

#---
# This function sets KAKADU_FOUND if variables are valid.
#--- 
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( Kakadu DEFAULT_MSG 
                                   KAKADU_LIBRARIES
                                   KAKADU_INCLUDE_DIR )

if( KAKADU_FOUND )
   if( NOT KAKADU_FIND_QUIETLY )
      message( STATUS "Found Kakadu..." )
   endif( NOT KAKADU_FIND_QUIETLY )
else( KAKADU_FOUND )
   if( NOT KAKADU_FIND_QUIETLY )
      message( WARNING "Could not find Kakadu" )
   endif( NOT KAKADU_FIND_QUIETLY )
endif( KAKADU_FOUND )

if( NOT KAKADU_FIND_QUIETLY )
   message( STATUS "KAKADU_INCLUDE_DIR: ${KAKADU_INCLUDE_DIR}" )
   message( STATUS "KAKADU_LIBRARY:     ${KAKADU_LIBRARY}" )
   message( STATUS "KAKADU_AUX_LIBRARY: ${KAKADU_AUX_LIBRARY}" )  
   message( STATUS "KAKADU_LIBRARIES:   ${KAKADU_LIBRARIES}" )
endif( NOT KAKADU_FIND_QUIETLY )
