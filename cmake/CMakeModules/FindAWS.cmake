#---
# File: FindAWS.cmake
#
# Find the native AWS(Amazon Web Services) cpp sdk includes and libraries.
#
# This module defines:
#
# AWS_INCLUDE_DIR, where to find Aws.h and S3Client.h.
# AWS_LIBRARIES, libraries to link against to use AWS.  Currently there are
# two looked for, aws-cpp-sdk-core and aws-cpp-sdk-s3 libraries.
# AWS_FOUND, True if found, false if one of the above are not found.
# 
# For ossim, typically geos will be system installed which should be found; 
# or found in the ossim 3rd party dependencies directory from a geos build 
# and install.  If the latter it will rely on CMAKE_INCLUDE_PATH and 
# CMAKE_LIBRARY_PATH having the path to the party dependencies directory.
# 
# NOTE: 
# This script is specialized for ossim.
#
# $Id$
#---

#---
# Find include path:
#---
find_path( AWS_CORE_INCLUDE_DIR aws/core/Aws.h
         PATHS 
            $ENV{AWS_DIR}/include
            ${AWS_DIR}/include)

# We need this so check for it:
find_path( AWS_S3_INCLUDE_DIR aws/s3/S3Client.h
          PATHS 
            $ENV{AWS_DIR}/include
            ${AWS_DIR}/include)

# Set the AWS_INCLUDE_DIR:
if( AWS_CORE_INCLUDE_DIR AND AWS_S3_INCLUDE_DIR )
   set( AWS_INCLUDE_DIR ${AWS_CORE_INCLUDE_DIR} CACHE STRING INTERNAL )
endif( AWS_CORE_INCLUDE_DIR AND AWS_S3_INCLUDE_DIR )

# Find AWS CPP SDK CORE library:
find_library( AWS_CORE_LIB NAMES aws-cpp-sdk-core )

# Find AWS CPP SDK S3 library:
find_library( AWS_S3_LIB NAMES aws-cpp-sdk-s3)

# Set the AWS_LIBRARY:
if( AWS_CORE_LIB AND AWS_S3_LIB )
   set( AWS_LIBRARIES ${AWS_CORE_LIB} ${AWS_S3_LIB} CACHE STRING INTERNAL )
endif(AWS_CORE_LIB AND AWS_S3_LIB )

#---
# This function sets AWS_FOUND if variables are valid.
#--- 
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( AWS DEFAULT_MSG 
                                   AWS_LIBRARIES
                                   AWS_INCLUDE_DIR )

if( AWS_FOUND )
   if( NOT AWS_FIND_QUIETLY )
      message( STATUS "Found AWS..." )
   endif( NOT AWS_FIND_QUIETLY )
else( AWS_FOUND )
   if( NOT AWS_FIND_QUIETLY )
      message( WARNING "Could not find AWS" )
   endif( NOT AWS_FIND_QUIETLY )
endif( AWS_FOUND )

if( NOT AWS_FIND_QUIETLY )
   message( STATUS "AWS_INCLUDE_DIR=${AWS_INCLUDE_DIR}" )
   message( STATUS "AWS_LIBRARIES=${AWS_LIBRARIES}" )
endif( NOT AWS_FIND_QUIETLY )
