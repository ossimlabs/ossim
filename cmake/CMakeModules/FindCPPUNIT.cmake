# - Find CPPUNIT
# Find the native CPPUNIT includes and library
# This module defines
#  CPPUNIT_INCLUDE_DIR, where to find jpeglib.h, etc.
#  CPPUNIT_LIBRARIES, the libraries needed to use CPPUNIT.
#  CPPUNIT_FOUND, If false, do not try to use CPPUNIT.
# also defined, but not for general use are
#  CPPUNIT_LIBRARY, where to find the CPPUNIT library.

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

FIND_PATH(CPPUNIT_INCLUDE_DIR cppunit/Test.h)

SET(CPPUNIT_NAMES ${CPPUNIT_NAMES} cppunit libcppunit)
FIND_LIBRARY(CPPUNIT_LIBRARY NAMES ${CPPUNIT_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set CPPUNIT_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CPPUNIT DEFAULT_MSG CPPUNIT_LIBRARY CPPUNIT_INCLUDE_DIR)

IF(CPPUNIT_FOUND)
  SET(CPPUNIT_LIBRARIES ${CPPUNIT_LIBRARY})
ENDIF(CPPUNIT_FOUND)

# Deprecated declarations.
SET (NATIVE_CPPUNIT_INCLUDE_PATH ${CPPUNIT_INCLUDE_DIR} )
IF(CPPUNIT_LIBRARY)
  GET_FILENAME_COMPONENT (NATIVE_CPPUNIT_LIB_PATH ${CPPUNIT_LIBRARY} PATH)
ENDIF(CPPUNIT_LIBRARY)

MARK_AS_ADVANCED(CPPUNIT_LIBRARY CPPUNIT_INCLUDE_DIR )
