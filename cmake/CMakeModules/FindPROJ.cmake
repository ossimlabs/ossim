###############################################################################
# CMake module to search for PROJ library
#
# On success, the macro sets the following variables:
# PROJ_FOUND       = if the library found
# PROJ_LIBRARY     = full path to the library
# PROJ_INCLUDE_DIR = where to find the library headers 
# also defined, but not for general use are
# PROJ_LIBRARY, where to find the PROJ library.
#
# Copyright (c) 2009 Mateusz Loskot <mateusz@loskot.net>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
###############################################################################

# Try to use OSGeo4W installation
IF(WIN32)
    SET(PROJ4_OSGEO4W_HOME "C:/OSGeo4W") 

    IF($ENV{OSGEO4W_HOME})
        SET(PROJ4_OSGEO4W_HOME "$ENV{OSGEO4W_HOME}") 
    ENDIF()
ENDIF(WIN32)

SET(_PROJ_SEARCH_PREFIXES)

# Windows OSGeo4W installation hint
LIST(APPEND _PROJ_SEARCH_PREFIXES ${PROJ_OSGEO4W_HOME})

# Allow users to point to a custom install via environment variables
FOREACH(_env_prefix PROJ_ROOT PROJ_PREFIX PROJ_DIR)
    IF(DEFINED ENV{${_env_prefix}} AND NOT "$ENV{${_env_prefix}}" STREQUAL "")
        LIST(APPEND _PROJ_SEARCH_PREFIXES $ENV{${_env_prefix}})
    ENDIF()
ENDFOREACH()

# Common Unix style prefixes (MacPorts, Homebrew, Linux)
LIST(APPEND _PROJ_SEARCH_PREFIXES
    /usr/local
    /usr
    /opt/local
    /opt/local/lib/proj
    /opt/local/lib/proj9
    /opt/homebrew
)

LIST(REMOVE_DUPLICATES _PROJ_SEARCH_PREFIXES)

FIND_PATH(PROJ_INCLUDE_DIR
    NAMES proj.h proj_experimental.h
    HINTS ${_PROJ_SEARCH_PREFIXES}
    PATH_SUFFIXES include include/proj include/proj9
    DOC "Path to PROJ library include directory")

SET(PROJ_NAMES ${PROJ_NAMES} proj proj_i)
FIND_LIBRARY(PROJ_LIBRARY
    NAMES ${PROJ_NAMES}
    HINTS ${_PROJ_SEARCH_PREFIXES}
    PATH_SUFFIXES lib lib64
    DOC "Path to PROJ library file")

# Handle the QUIETLY and REQUIRED arguments and set PROJ_FOUND to TRUE
# if all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PROJ DEFAULT_MSG PROJ_LIBRARY PROJ_INCLUDE_DIR)

IF(PROJ_FOUND)
  SET(PROJ_LIBRARIES ${PROJ_LIBRARY})
ENDIF()
