PROJECT(ossim)
############################################################
# From https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
#
# Use, don't skip, the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH FALSE)

# When building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif("${isSystemDir}" STREQUAL "-1")
#################################################################

SET(CMAKE_MODULE_PATH "${${PROJECT_NAME}_SOURCE_DIR}/cmake/CMakeModules;${CMAKE_MODULE_PATH}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

Message("################## Setting up OSSIM core library #########################")

IF(NOT OSSIM_BUILD_ADDITIONAL_OSSIM_DIRECTORIES)
   SET(OSSIM_BUILD_ADDITIONAL_OSSIM_DIRECTORIES "" CACHE PATH "Specify full paths separated by ; to additional OSSIM applications you wish to include into the ossim build framework.  These will have variables setup for the ossim includes" FORCE)
ENDIF(NOT OSSIM_BUILD_ADDITIONAL_OSSIM_DIRECTORIES)

cmake_minimum_required(VERSION 2.8)

SET(CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/ossim/cmake/CMakeModules" ${CMAKE_MODULE_PATH})

include(CMakeDetermineSystem)
INCLUDE(OssimVersion)
INCLUDE(OssimCommonVariables)

# Expose some build options
set(LIB_NAME ossim CACHE STRING "Name of ossim libray (default is ossim).")

OPTION(BUILD_OSSIM_CURL_SUPPORT "Set to ON to build OSSIM with curl support.  Use OFF to turn off curl support." ON)
OPTION(BUILD_OSSIM_FREETYPE_SUPPORT "Set to ON to build OSSIM with freetype support.  Use OFF to turn off freetype support." ON)
OPTION(BUILD_OSSIM_HDF5_SUPPORT "Set to ON to build OSSIM with HDF5 support.  Use OFF to turn off HDF5 support." OFF)
OPTION(BUILD_OSSIM_ID_SUPPORT "Set to ON to build OSSIM GIT ID support into the library.  Use OFF to turn off ID support." ON)
OPTION(BUILD_OSSIM_MPI_SUPPORT "Set to ON to build OSSIM with MPI support.  Use OFF to turn off MPI support." OFF)
OPTION(BUILD_OSSIM_PROJ_SUPPORT "Set to ON to build OSSIM with proj support.  Use OFF to turn off proj support." ON)
OPTION(BUILD_OSSIM_SQLITE_SUPPORT "Set to ON to build OSSIM with sqlite support.  Use OFF to turn off sqlite support." ON)

OPTION(BUILD_OSSIM_APPS "Set to ON to build OSSIM applications." ON)
OPTION(BUILD_OSSIM_CURL_APPS "Set to ON to build ossim curl dependent apps. Use ON to enable." OFF)
OPTION(BUILD_OSSIM_TESTS "Set to ON to build OSSIM unit/functional tests." ON)


###################################################################################
# Include the Utilities in the root make
###################################################################################
INCLUDE(OssimUtilities)

INCLUDE(CheckIncludeFile)
CHECK_INCLUDE_FILE("dirent.h" CMAKE_HAVE_DIRENT_H)
CHECK_INCLUDE_FILE("unistd.h" CMAKE_HAVE_UNISTD_H)
CHECK_INCLUDE_FILE("getopt.h" CMAKE_HAVE_GETOPT_H)
CHECK_INCLUDE_FILE("fcntl.h" CMAKE_HAVE_FCNTL_H)
CHECK_INCLUDE_FILE("dbmalloc.h" CMAKE_HAVE_DBMALLOC_H)
CHECK_INCLUDE_FILE("malloc.h" CMAKE_HAVE_MALLOC_H)
CHECK_INCLUDE_FILE("dlfcn.h" CMAKE_HAVE_DLFCN_H)

# Set our include paths:
include_directories( ${PROJECT_SOURCE_DIR}/include )
include_directories( ${PROJECT_BINARY_DIR}/include )
include_directories( ${OSSIM_INCLUDE_DIR} )
include_directories( ${OSSIM_INCLUDE_DIR}/ossim )

# Stores list of libs to link with. Initialized throughout.
set( ossimDependentLibs )

#---
# Find required and optional packages and add their include paths and libraries:
#---

# GEOS - Required:
find_package( GEOS )
if( GEOS_FOUND )
   include_directories( ${GEOS_INCLUDE_DIR} )
   set( ossimDependentLibs ${ossimDependentLibs} ${GEOS_C_LIBRARY} )
else( GEOS_FOUND )
   message( FATAL_ERROR "Could not find required geos package!" )
endif( GEOS_FOUND )

# JPEG - Required package:
find_package( JPEG )
if( JPEG_FOUND )
   include_directories( ${JPEG_INCLUDE_DIR} )
   set( ossimDependentLibs ${ossimDependentLibs} ${JPEG_LIBRARY} )
else( JPEG_FOUND )
   message( FATAL_ERROR "Could not find required jpeg package!" )
endif( JPEG_FOUND )

# JSONCPP - Required:
find_package( JsonCpp )
if( JSONCPP_FOUND )
   include_directories( ${JSONCPP_INCLUDE_DIR} )
   set( ossimDependentLibs ${ossimDependentLibs} ${JSONCPP_LIBRARY} )
else( JSONCPP_FOUND )
   message( FATAL_ERROR "Could not find required JsonCpp package!" )
endif( JSONCPP_FOUND )

# ZLIB - Optional, but probably required by other libraries, e.g. libtiff:
set( OSSIM_HAS_LIBZ 0 )
find_package( ZLIB )
if ( ZLIB_FOUND )
   include_directories( ${ZLIB_INCLUDE_DIR} )
   set( ossimDependentLibs ${ossimDependentLibs} ${ZLIB_LIBRARY} )
   set( OSSIM_HAS_LIBZ 1 )
else ( ZLIB_FOUND )
   message( WARNING "Could not find optional zlib package!" )
endif ( ZLIB_FOUND )

# TIFF - Required:
find_package( TIFF )
if( TIFF_FOUND )
   include_directories( ${TIFF_INCLUDE_DIR} )
   set( ossimDependentLibs ${ossimDependentLibs} ${TIFF_LIBRARY} )
else( TIFF_FOUND )
   message( FATAL_ERROR "Could not find required tiff package!" )
endif( TIFF_FOUND )

# Curl - Required by proj which is required by geotiff:
if( BUILD_OSSIM_CURL_SUPPORT )
   find_package( CURL )
   if( CURL_FOUND )
      include_directories( ${CURL_INCLUDE_DIR} )
      set(  ossimDependentLibs ${ossimDependentLibs} ${CURL_LIBRARIES} )
   else( CURL_FOUND )
      message(FATAL_ERROR "Could not find curl")
   endif( CURL_FOUND )
endif( BUILD_OSSIM_CURL_SUPPORT )

# SQLite - Required by proj which is required by geotiff:
if ( BUILD_OSSIM_SQLITE_SUPPORT ) 
   find_package(SQLITE)
   if(SQLITE_FOUND)
      include_directories( ${SQLITE_INCLUDE_DIR} )
      set( ossimDependentLibs ${ossimDependentLibs} ${SQLITE_LIBRARY})
   else(SQLITE_FOUND)
      message(FATAL_ERROR "Could not find sqlite")
   endif(SQLITE_FOUND)
endif( BUILD_OSSIM_SQLITE_SUPPORT )

# PROJ - Required by geotiff:
if ( BUILD_OSSIM_PROJ_SUPPORT )
   find_package( PROJ )
   if( PROJ_FOUND )
      include_directories( ${PROJ_INCLUDE_DIR} )
      set( ossimDependentLibs ${ossimDependentLibs} ${PROJ_LIBRARY} )
   else( PROJ_FOUND )
      message( WARNING "Could not find proj package! May be needed for libgeotiff." )
   endif( PROJ_FOUND )
endif( BUILD_OSSIM_PROJ_SUPPORT )

# GEOFIFF - Required:
set( OSSIM_HAS_GEOTIFF 0 )
find_package( GEOTIFF )
if( GEOTIFF_FOUND )
   include_directories( ${GEOTIFF_INCLUDE_DIR} )
   set( ossimDependentLibs ${ossimDependentLibs} ${GEOTIFF_LIBRARY} )
   set( OSSIM_HAS_GEOTIFF 1 )
else( GEOTIFF_FOUND )
   message( FATAL_ERROR "Could not find required geotiff package!" )
endif( GEOTIFF_FOUND )

# GEOTRANS - Required: (GeoTrans 3.4.x port future...)
# find_package( Geotrans )
# if( GEOTRANS_FOUND )
#   include_directories( ${GEOTRANS_INCLUDE_DIR} )
#   set( ossimDependentLibs ${ossimDependentLibs} ${GEOTRANS_LIBRARY} )
# else( GEOTRANS_FOUND )
#   message( FATAL_ERROR "Could not find required geotrans package!" )
# endif( GEOTRANS_FOUND )


# This caused a core dump on ossim executables running in jenins pipeline (ossim-test-dev)
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
  find_library( DL_LIBRARY dl )
  if ( DL_LIBRARY )
    set( ossimDependentLibs ${ossimDependentLibs} ${DL_LIBRARY} )
  else( DL_LIBRARY )
    # TODO:  Is this required on unix?  If so move to the required section.
    message( FATAL_ERROR "Could not find dl library!" )
  endif( DL_LIBRARY )
endif() # if (CMAKE_SYSTEM_NAME MATCHES "Linux")

#if( UNIX )
#   find_library( DL_LIBRARY dl )
#  if ( DL_LIBRARY )
#      set( ossimDependentLibs ${ossimDependentLibs} ${DL_LIBRARY} )
#   else( DL_LIBRARY )
#      # TODO:  Is this required on unix?  If so move to the required section.
#      message( FATAL_ERROR "Could not find dl library!" )
#   endif( DL_LIBRARY )
#endif( UNIX )

# FREETYPE - Optional:
set( OSSIM_HAS_FREETYPE 0 )
if( BUILD_OSSIM_FREETYPE_SUPPORT )
   find_package(Freetype)
   if( FREETYPE_FOUND )
      include_directories( ${FREETYPE_INCLUDE_DIRS} )
      set( ossimDependentLibs ${ossimDependentLibs} ${FREETYPE_LIBRARIES} )
      set( OSSIM_HAS_FREETYPE 1 )
   else( FREETYPE_FOUND )
      message( WARNING "Could not find optional freetype package!" )
   endif( FREETYPE_FOUND )
endif( BUILD_OSSIM_FREETYPE_SUPPORT )

# MPI - Optional:
set( OSSIM_HAS_MPI 0 )
if( BUILD_OSSIM_MPI_SUPPORT )
   find_package(MPI)
   if ( MPI_FOUND )
      # /usr/share/cmake/Modules/FindMPI.cmae are not consistent in terminology,
      # so we include multiple list names, knowing some will be empty
      include_directories( ${MPI_INCLUDE_DIR} )
      include_directories( ${MPI_CXX_INCLUDE_PATH} ${MPI_CXX_INCLUDE_DIRS} )
      include_directories( ${MPI_C_INCLUDE_PATH} ${MPI_C_INCLUDE_DIRS} )
      set( ossimDependentLibs ${ossimDependentLibs} ${MPI_LIBRARIES} )
      set( OSSIM_HAS_MPI 1 )
   else ( MPI_FOUND )
      message( WARNING "Could not find optional MPI package!" )
   endif ( MPI_FOUND )
endif( BUILD_OSSIM_MPI_SUPPORT )

# HDF5 - Optional:
set( OSSIM_HAS_HDF5 0 )
if (BUILD_OSSIM_HDF5_SUPPORT)
  message( STATUS "Native HDF5 support requested. Searching for HDF5 library..." )
  find_package( HDF5A )
  if ( HDF5A_FOUND)
     include_directories( ${HDF5A_INCLUDE_DIR} )
     set( ossimDependentLibs ${ossimDependentLibs} ${HDF5A_LIBRARIES} )
     set( OSSIM_HAS_HDF5 1 )
  else ( HDF5A_FOUND )
     message( WARNING "Could not find optional HDF5A package. Excluding native support." )
  endif ( HDF5A_FOUND )
else (BUILD_OSSIM_HDF5_SUPPORT)
  message( STATUS "Native HDF5 support was not requested. If you want HDF5 support, set the environment BUILD_OSSIM_HDF5_SUPPORT=ON..." )
endif ( BUILD_OSSIM_HDF5_SUPPORT )

find_package( Threads )
set( ossimDependentLibs ${ossimDependentLibs} ${CMAKE_THREAD_LIBS_INIT} )

#---
# Call the OSSIM macros in OssimUtilities.cmake
#---
if ( NOT OSSIM_BUILD_DATE )
   TODAYS_DATE(OSSIM_BUILD_DATE)
endif( NOT OSSIM_BUILD_DATE )
SET(OSSIM_GIT_REVISION_NUMBER "UNKNOWN")
GET_GIT_REVISION()
if ( NOT ${Project_WC_REVISION} EQUAL 0 )
   set( OSSIM_GIT_REVISION_NUMBER ${Project_WC_REVISION} )
endif()

#####################################################################################
# Call the configure files for ossimConfig and ossimVersion setup
#####################################################################################
include(CheckTypeSize)
check_type_size("short int" SIZE_OF_SHORT_INT)
check_type_size("int" SIZE_OF_INT)
check_type_size("long" SIZE_OF_LONG)
check_type_size("long long" SIZE_OF_LONG_LONG)
check_type_size("float" SIZE_OF_FLOAT)
check_type_size("double" SIZE_OF_DOUBLE)
check_type_size("long double" SIZE_OF_LONG_DOUBLE)

check_type_size("unsigned short int" SIZE_OF_UNSIGNED_SHORT_INT)
check_type_size("unsigned int" SIZE_OF_UNSIGNED_INT)
check_type_size("unsigned long" SIZE_OF_UNSIGNED_LONG)
check_type_size("unsigned long long" SIZE_OF_UNSIGNED_LONG_LONG)

set(OSSIM_VERSION_NUMBER "\"${OSSIM_VERSION}\"")
set(OSSIM_BUILD_DATE "\"${OSSIM_BUILD_DATE}\"")
set(OSSIM_REVISION "\"${OSSIM_GIT_REVISION_NUMBER}\"")

# Setting of OSSIM_ID to variable expanded $Id$ results.  Embedded troughout code.
set(OSSIM_ID_ENABLED   1)
IF(NOT BUILD_OSSIM_ID_SUPPORT)
   set(OSSIM_ID_ENABLED 0)
ENDIF(NOT BUILD_OSSIM_ID_SUPPORT)

SET(OSSIM_CONFIGURE_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/ossim/ossimConfig.h")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/src/ossimConfig.h.in"
         "${OSSIM_CONFIGURE_HEADER}")
SET(OSSIM_VERSION_HEADER_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/src/ossimVersion.h.in")
SET(OSSIM_VERSION_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/ossim/ossimVersion.h")
CONFIGURE_FILE("${OSSIM_VERSION_HEADER_CONFIG}"
               "${OSSIM_VERSION_HEADER}")

##################################################################################################
# Point to the CMakeLists.txt in the src dir. OLK: Removed unnecessary src/ossim structure and
# replaced with simply src.
##################################################################################################
subdirs(src)

set(TARGET_COMMON_LIBRARIES ossim)

##################################################################################################
#
# Setup install destinations for the shared files:
# 1) Projection CSV files:
#
##################################################################################################
FILE(GLOB ossim_projection_codes_csv ${${PROJECT_NAME}_SOURCE_DIR}/share/ossim/projection/*.csv ${${PROJECT_NAME}_SOURCE_DIR}/share/ossim/projection/*.txt)
INSTALL(FILES ${ossim_projection_codes_csv} DESTINATION share/ossim/projection COMPONENT ossim)

# 2) Utility API JSON files
FILE(GLOB ossim_util_api_json ${${PROJECT_NAME}_SOURCE_DIR}/share/ossim/util/*.json)
INSTALL(FILES ${ossim_util_api_json} DESTINATION share/ossim/util COMPONENT ossim)

# 3) Fonts
FILE(GLOB ossim_fonts ${${PROJECT_NAME}_SOURCE_DIR}/share/ossim/fonts/*.ttf)
INSTALL(FILES ${ossim_fonts} DESTINATION share/ossim/fonts COMPONENT ossim)

###################### OUTPUT GENERAL VARIABLE SETTINGS #######################

MESSAGE( STATUS "OSSIM_REVISION                  = ${OSSIM_REVISION}" )
MESSAGE( STATUS "FREETYPE_LIBRARY                = ${FREETYPE_LIBRARIES}" )
MESSAGE( STATUS "FREETYPE_INCLUDE                = ${FREETYPE_INCLUDE_DIRS}" )
MESSAGE( STATUS "GEOTIFF_LIBRARY                 = ${GEOTIFF_LIBRARIES}" )
MESSAGE( STATUS "GEOTIFF_INCLUDE                 = ${GEOTIFF_INCLUDE_DIR}" )
MESSAGE( STATUS "GEOTRANS_LIBRARY                = ${GEOTRANS_LIBRARY}" )
MESSAGE( STATUS "GEOTRANS_INCLUDE                = ${GEOTRANS_INCLUDE_DIR}" )
MESSAGE( STATUS "GEOS_C_LIBRARY                  = ${GEOS_C_LIBRARY}" )
MESSAGE( STATUS "GEOS_INCLUDE                    = ${GEOS_INCLUDE_DIR}" )
MESSAGE( STATUS "JPEG_LIBRARY                    = ${JPEG_LIBRARIES}" )
MESSAGE( STATUS "JPEG_INCLUDE                    = ${JPEG_INCLUDE_DIR}" )
MESSAGE( STATUS "OSSIM DEPENDENT LIBRARIES       = ${ossimDependentLibs}" )
MESSAGE( STATUS "OSSIM_HAS_HDF5                  = ${OSSIM_HAS_HDF5}" )

#-----------------------------------------------------------------------------
# uninstall target
#-----------------------------------------------------------------------------
#OSSIM_ADD_COMMON_MAKE_UNINSTALL()

IF(BUILD_OSSIM_APPS)
   add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/apps ${CMAKE_CURRENT_BINARY_DIR}/apps)
ENDIF()

#---
# Building curl dependant apps optional. default=OFF
#---
if(BUILD_OSSIM_CURL_APPS)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/apps/curl_apps)
endif()

IF(BUILD_OSSIM_TESTS)
   add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test/src ${CMAKE_CURRENT_BINARY_DIR}/test/src)
ENDIF()

message("################## end of OSSIM core library setup #########################\n")
