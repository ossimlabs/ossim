########################################################################
#
# Find the JSON CPP include and library.
#
# This module defines:
#
# JSONCPP_INCLUDE_DIR, where to find json.h, etc.
# JSONCPP_LIBRARY, library to link against to use jsoncpp.
# JSONCPP_FOUND, True if found, false if one of the above are not found.
# USE_OSSIM_JSONCPP, Set to "YES" if no external package detected.
#    Directs cmake to include the jsoncpp subirectories in ossim build.
#
########################################################################

set(JSONCPP_FOUND "NO")
set(OSSIM_HAS_JSONCPP 0)

find_path( JSONCPP_INCLUDE_DIR json/json.h
          PATHS "${CMAKE_INSTALL_PREFIX}" "/usr/include/jsoncpp" )

find_library( JSONCPP_LIBRARY NAMES "jsoncpp")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( JSONCPP DEFAULT_MSG 
                                   JSONCPP_LIBRARY 
                                   JSONCPP_INCLUDE_DIR )

if (JSONCPP_FOUND)
   set(OSSIM_HAS_JSONCPP 1)
endif (JSONCPP_FOUND)

MARK_AS_ADVANCED(JSONCPP_LIBRARY JSONCPP_INCLUDE_DIR )
