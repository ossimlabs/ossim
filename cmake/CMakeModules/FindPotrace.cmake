#---
# File: FindPotrace.cmake
#
# Find the Potrace includes and libraries.
#
# This module defines:
#
# POTRACE_INCLUDE_DIR, where to find potracelib.h, etc.
# POTRACE_LIBRARIES, libraries to link against to use Potrace and rialto.
# POTRACE_FOUND, True if found, false if one of the above are not found.
# 
find_path( POTRACE_INCLUDE_DIR potracelib.h
           PATHS 
           ${OSSIM_DEPENDENCIES}/include
           /usr/include
           /usr/local/include
           /usr/local/ossim/include )

find_library( POTRACE_LIBRARY
      NAMES "potrace"
      PATHS
      ${OSSIM_DEPENDENCIES}/lib
      /usr/lib64
      /usr/lib
     /usr/local/lib
   )


message( STATUS "POTRACE_INCLUDE_DIR             = ${POTRACE_INCLUDE_DIR}" )
message( STATUS "POTRACE_LIBRARY                 = ${POTRACE_LIBRARY}" )

set(POTRACE_FOUND "NO")

if (POTRACE_INCLUDE_DIR AND POTRACE_LIBRARY )

   set(POTRACE_FOUND "YES")
   set(POTRACE_LIBRARIES ${POTRACE_LIBRARY} )
   message( STATUS "POTRACE_LIBRARIES = ${POTRACE_LIBRARIES}" )

else(POTRACE_INCLUDE_DIR AND POTRACE_LIBRARY)

   message( WARNING "Could not find Potrace" )

endif(POTRACE_INCLUDE_DIR AND POTRACE_LIBRARY)


