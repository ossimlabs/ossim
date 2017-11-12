#---
# File: FindPodofo.cmake
#
# Find the native podofo includes and library
#
# This module defines:
#  PODOFO_INCLUDE_DIR, where to find tiff.h, etc.
#  PODOFO_LIBRARY, library to link against to use PODOFO.
#  PODOFO_FOUND, If false, do not try to use PODOFO.
# 
# $Id$
#---

# Find include path:
find_path(PODOFO_INCLUDE_DIR podofo/podofo.h )

# Find library:
find_library(PODOFO_LIBRARY NAMES podofo )

#---
# This function sets PODOFO_FOUND if variables are valid.
#--- 
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PODOFO  DEFAULT_MSG  PODOFO_LIBRARY  PODOFO_INCLUDE_DIR)
