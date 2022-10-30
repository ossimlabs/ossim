# - Find JPEG
# Find the native JPEG includes and library
# This module defines
#  SHP_INCLUDE_DIR, where to find jpeglib.h, etc.
#  SHP_LIBRARIES, the libraries needed to use JPEG.
#  SHP_FOUND, If false, do not try to use JPEG.
# also defined, but not for general use are
#  SHP_LIBRARY, where to find the JPEG library.

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

FIND_PATH(SHP_INCLUDE_DIR shapefil.h)

SET(SHP_RELEASE_NAMES ${SHP_NAMES} shp libshp)
SET(SHP_DEBUG_NAMES ${SHP_NAMES} shpd libshpd)

# Added x86_64-linux-gnu path for Ubuntu install
FIND_LIBRARY(SHP_LIBRARY_RELEASE NAMES ${SHP_RELEASE_NAMES})
FIND_LIBRARY(SHP_LIBRARY_DEBUG NAMES ${SHP_DEBUG_NAMES})

include(SelectLibraryConfigurations)
select_library_configurations(SHP)

# handle the QUIETLY and REQUIRED arguments and set SHP_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SHP DEFAULT_MSG SHP_LIBRARY SHP_INCLUDE_DIR)

IF(SHP_FOUND)
  SET(SHP_LIBRARIES ${SHP_LIBRARY})
ENDIF(SHP_FOUND)

# Deprecated declarations.
SET (NATIVE_SHP_INCLUDE_PATH ${SHP_INCLUDE_DIR} )
IF(SHP_LIBRARY)
    GET_FILENAME_COMPONENT (NATIVE_SHP_LIB_PATH ${SHP_LIBRARY_RELEASE} PATH)
ENDIF(SHP_LIBRARY)

MARK_AS_ADVANCED(SHP_LIBRARY SHP_INCLUDE_DIR )
