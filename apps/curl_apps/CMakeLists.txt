message( "************** Begin: CMAKE SETUP FOR ossim curl apps ******************" )

cmake_minimum_required (VERSION 2.8)

project(ossimcore)

set(requiredLibs)

# Get the library suffix for lib or lib64.
get_property(LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)       
if(LIB64)
   set(LIBSUFFIX 64)
else()
   set(LIBSUFFIX "")
endif()

# OpenThreads - Required:
find_package( OpenThreads )
if( OPENTHREADS_FOUND )
   include_directories( ${OPENTHREADS_INCLUDE_DIR} )
   set( requiredLibs ${requiredLibs} ${OPENTHREADS_LIBRARY} )
else( OPENTHREADS_FOUND )
   message( FATAL_ERROR "Could not find required OpenThreads package!" )
endif( OPENTHREADS_FOUND )

find_package( CURL )
if( CURL_FOUND )
   include_directories( ${CURL_INCLUDE_DIR} )
   set( requiredLibs ${requiredLibs} ${CURL_LIBRARY} )
else( CURL_FOUND )
   message(FATAL_ERROR "Could not find curl") 
endif( CURL_FOUND )

# Requires ossim:
find_package(ossim)
if (OSSIM_FOUND)
   include_directories( ${OSSIM_INCLUDE_DIR} )
   set(requiredLibs ${requiredLibs} ${OSSIM_LIBRARIES} )
else(OSSIM_FOUND)
   message(FATAL_ERROR "Could not find ossim")
endif(OSSIM_FOUND)

# Add our include...
include_directories(".")

message( STATUS "CURL_INCLUDE_DIR    = ${CURL_INCLUDE_DIR}" )
message( STATUS "CURL_LIBRARY        = ${CURL_LIBRARY}" )
message( STATUS "OPENTHREADS_LIBRARY = ${OPENTHREADS_LIBRARY}" )
message( STATUS "OPENTHREADS_INCLUDE = ${OPENTHREADS_INCLUDE_DIR}" )
message( STATUS "OSSIM_INCLUDE_DIR   = ${OSSIM_INCLUDE_DIR}" )
message( STATUS "OSSIM_LIBRARIES     = ${OSSIM_LIBRARIES}" )
message( STATUS "Required libs       = ${requiredLibs}" )

# ---
# omar-data-mgr app:
# ---

# Add the executable:
add_executable(omar-data-mgr omar-data-mgr.cpp omarDataMgrUtil.cpp )

# Set the output dir:
set_target_properties(omar-data-mgr 
                      PROPERTIES 
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# omar-data-mgr application links with required libs.
target_link_libraries( omar-data-mgr ${requiredLibs} )

# Install app:
install( TARGETS omar-data-mgr
         DESTINATION bin )

message( "************** End: CMAKE SETUP FOR ossim curl apps ******************" )
