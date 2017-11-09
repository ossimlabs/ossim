#---
# File: FindSQLITE.cmake
#
# Find the native SQLite includes and libraries.
#
# This module defines:
#
# SQLITE_INCLUDE_DIR, where to find sqlite3.h, etc.
# SQLITE_LIBRARY, libraries to link against to use SQLITE.
# SQLITE_FOUND, True if found, false if one of the above are not found.
# 
# For ossim, typically SQLite will be system installed which should be found; 
# or found in the ossim 3rd party dependencies directory from a SQLite build 
# and install.  If the latter it will rely on CMAKE_INCLUDE_PATH and 
# CMAKE_LIBRARY_PATH having the path to the party dependencies directory.
# 
# $Id$
#---

#---
# Find include path:
#---
find_path( SQLITE_INCLUDE_DIR sqlite3.h
           PATHS 
           /usr/include
           /usr/local/include )

# Find SQLITE library:
find_library( SQLITE_LIB NAMES sqlite3
              PATHS 
              /usr/lib64 
              /usr/lib 
              /usr/local/lib )

# Set the SQLITE_LIBRARY:
if( SQLITE_LIB )
   set( SQLITE_LIBRARY ${SQLITE_LIB} CACHE STRING INTERNAL )
endif(SQLITE_LIB )

#---
# This function sets SQLITE_FOUND if variables are valid.
#--- 
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( SQLITE DEFAULT_MSG 
                                   SQLITE_LIBRARY 
                                   SQLITE_INCLUDE_DIR )

if( SQLITE_FOUND )
   if( NOT SQLITE_FIND_QUIETLY )
      message( STATUS "Found SQLITE..." )
   endif( NOT SQLITE_FIND_QUIETLY )
else( SQLITE_FOUND )
   if( NOT SQLITE_FIND_QUIETLY )
      message( WARNING "Could not find SQLITE" )
   endif( NOT SQLITE_FIND_QUIETLY )
endif( SQLITE_FOUND )

if( NOT SQLITE_FIND_QUIETLY )
   message( STATUS "SQLITE_INCLUDE_DIR=${SQLITE_INCLUDE_DIR}" )
   message( STATUS "SQLITE_LIBRARY=${SQLITE_LIBRARY}" )
endif( NOT SQLITE_FIND_QUIETLY )
