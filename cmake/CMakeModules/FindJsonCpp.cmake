####################################################################################################
#
# Find the JSON CPP include and library.
#
# This module references:
#   OSSIM_DEV_HOME
#   OSSIM_INSTALL_PREFIX
#   USE_OSSIM_JSONCPP
#
# This module defines:
#   JSONCPP_INCLUDE_DIR, where to find json.h, etc.
#   JSONCPP_LIBRARY, library to link against to use jsoncpp.
#   JSONCPP_FOUND, True if found, false if one of the above are not found.
#   USE_OSSIM_JSONCPP, Set to "YES" if not defined previously, and amalgamated header found in OSSIM
#
####################################################################################################

include(FindPackageHandleStandardArgs)

if(NOT DEFINED USE_OSSIM_JSONCPP)
   # No preference on amalgamated versus installed package, look for header file in OSSIm
   # and use that amalgamated form if available:
   find_path( JSONCPP_INCLUDE_DIR json/json.h
         PATHS
         ${OSSIM_DEV_HOME}/ossim/include/ossim
         ${OSSIM_INSTALL_PREFIX}/include/ossim NO_DEFAULT_PATH)
   if (EXISTS ${JSONCPP_INCLUDE_DIR})
      set(USE_OSSIM_JSONCPP "ON")
   endif()
endif()

if (USE_OSSIM_JSONCPP)
   # The use of the amalgamated form is indicated (or if not, the header file was found):
   find_path( JSONCPP_INCLUDE_DIR json/json.h
         PATHS
         ${OSSIM_DEV_HOME}/ossim/include/ossim
         ${OSSIM_INSTALL_PREFIX}/include/ossim NO_DEFAULT_PATH)
   message("-- JSONCPP: Using amalgamated form in ossim package.")
   set(JSONCPP_LIBRARY "")
   find_package_handle_standard_args( JsonCpp DEFAULT_MSG JSONCPP_INCLUDE_DIR )

else(USE_OSSIM_JSONCPP)
   # The use of the amalgamated form was explicitly rejected. Need to look for installed package:
   set(JSONCPP_FOUND "NO")
   set(OSSIM_HAS_JSONCPP 0)
   find_path( JSONCPP_INCLUDE_DIR json/json.h
         PATHS "${CMAKE_INSTALL_PREFIX}"
         "${CMAKE_INSTALL_PREFIX}/include/jsoncpp"
         "/usr/include/jsoncpp" )
   find_library( JSONCPP_LIBRARY NAMES "jsoncpp")
   find_package_handle_standard_args( JSONCPP DEFAULT_MSG JSONCPP_LIBRARY JSONCPP_INCLUDE_DIR )
endif(USE_OSSIM_JSONCPP)

if (JSONCPP_FOUND)
   set(OSSIM_HAS_JSONCPP 1)
endif (JSONCPP_FOUND)

MARK_AS_ADVANCED(JSONCPP_LIBRARY JSONCPP_INCLUDE_DIR )
