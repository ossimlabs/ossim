# - Find MrSid
# Find the native MrSid includes and library
# This module defines
#  MRSID_INCLUDE_DIR, where to header files.
#  MRSID_LIBRARIES, the libraries needed to use mrsid.
#  MRSID_FOUND, If false, do not try to use mrsid.

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

#---
# Find the include dirs:
#---
FIND_PATH( MRSID_RASTER_INCLUDE_DIR MrSIDImageReader.h
           ${MRSID_DIR}/Raster_DSDK/include )

FIND_PATH( MRSID_LIDAR_INCLUDE_DIR lidar/MG4PointReader.h
           ${MRSID_DIR}/Lidar_DSDK/include )

#---
# Find the Geo_DSDK (decode) library.
#----
# SET(MRSID_RASTER_DSDK_C_NAMES lti_dsdk_cdll ltidsdk_c libltidsdkc)
# FIND_LIBRARY( MRSID_RASTER_DSDK_C_LIBRARY 
#	      NAMES ${MRSID_RASTER_DSDK_C_NAMES} 
#              PATHS
#              ${MRSID_DIR}/Raster_DSDK/lib )

SET(MRSID_RASTER_DSDK_NAMES lti_dsdk ltidsdk libltidsdk)
FIND_LIBRARY( MRSID_RASTER_DSDK_LIBRARY 
	      NAMES ${MRSID_RASTER_DSDK_NAMES} 
              PATHS
              ${MRSID_DIR}/Raster_DSDK/lib )

SET(MRSID_LIDAR_DSDK_NAMES lti_lidar_dsdk liblti_lidar_dsdk )
FIND_LIBRARY( MRSID_LIDAR_DSDK_LIBRARY NAMES ${MRSID_LIDAR_DSDK_NAMES} 
              PATHS
              ${MRSID_DIR}/Lidar_DSDK/lib )

# handle the QUIETLY and REQUIRED arguments and set MRSID_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(MRSID DEFAULT_MSG MRSID_RASTER_INCLUDE_DIR MRSID_LIDAR_INCLUDE_DIR MRSID_RASTER_DSDK_LIBRARY MRSID_LIDAR_DSDK_LIBRARY)

# FIND_PACKAGE_HANDLE_STANDARD_ARGS(MRSID DEFAULT_MSG MRSID_RASTER_INCLUDE_DIR MRSID_LIDAR_INCLUDE_DIR MRSID_RASTER_DSDK_LIBRARY MRSID_RASTER_DSDK_C_LIBRARY MRSID_LIDAR_DSDK_LIBRARY)

IF(MRSID_FOUND)
   # Combine the includes into MRSID_INCLUDE_DIR variable:
   set(MRSID_INCLUDE_DIR ${MRSID_RASTER_INCLUDE_DIR} ${MRSID_LIDAR_INCLUDE_DIR})

   # Combine the libraries into MRSID_LIBRARY variable:
   set(MRSID_LIBRARY ${MRSID_RASTER_DSDK_LIBRARY} ${MRSID_LIDAR_DSDK_LIBRARY})

   # set(MRSID_LIBRARY ${MRSID_RASTER_DSDK_C_LIBRARY} ${MRSID_RASTER_DSDK_LIBRARY} ${MRSID_LIDAR_DSDK_LIBRARY})

  #---
  # Find the Geo_ESDK (encode) library.
  #----
  SET(MRSID_ESDK_NAMES ${MRSID_ESDK_NAMES} ltiesdk libltiesdk)
  FIND_LIBRARY(MRSID_ESDK_LIBRARY NAMES ${MRSID_ESDK_NAMES})

  #---
  # Find the two includes need for the writer.
  #---
  FIND_FILE(MG2IMAGEWRITER_H MG2ImageWriter.h ${MRSID_INCLUDE_DIR})
  FIND_FILE(MG3IMAGEWRITER_H MG3ImageWriter.h ${MRSID_INCLUDE_DIR})

  FIND_PACKAGE_HANDLE_STANDARD_ARGS(MRSID_WRITE DEFAULT_MSG MRSID_ESDK_LIBRARY MG2IMAGEWRITER_H MG3IMAGEWRITER_H)  

  IF(MRSID_WRITE_FOUND)
    add_definitions(-DOSSIM_ENABLE_MRSID_WRITE)
    SET(MRSID_LIBRARY ${MRSID_DSDK_LIBRARY} ${MRSID_ESDK_LIBRARY})
  ENDIF(MRSID_WRITE_FOUND)
  SET(MRSID_LIBRARIES ${MRSID_LIBRARY})
ENDIF(MRSID_FOUND)

MARK_AS_ADVANCED(MRSID_LIBRARIES MRSID_INCLUDE_DIR )
