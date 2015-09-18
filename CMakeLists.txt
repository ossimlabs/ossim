PROJECT(ossim)

Message("################## Setting up OSSIM core library #########################")

IF(NOT OSSIM_BUILD_ADDITIONAL_OSSIM_DIRECTORIES)
   SET(OSSIM_BUILD_ADDITIONAL_OSSIM_DIRECTORIES "" CACHE PATH "Specify full paths separated by ; to additional OSSIM applications you wish to include into the ossim build framework.  These will have variables setup for the ossim includes" FORCE)
ENDIF(NOT OSSIM_BUILD_ADDITIONAL_OSSIM_DIRECTORIES)

IF(NOT APPLE)
  cmake_minimum_required(VERSION 2.6)
ELSE(NOT APPLE)
  cmake_minimum_required(VERSION 2.8)
ENDIF(NOT APPLE)

INCLUDE(OssimVersion)
INCLUDE(OssimCommonVariables)

# Expose some build options
OPTION(BUILD_OSSIM_FREETYPE_SUPPORT "Set to ON to build OSSIM with freetype support.  Use OFF to turn off freetype support." ON)
OPTION(BUILD_OSSIM_MPI_SUPPORT "Set to ON to build OSSIM with MPI support.  Use OFF to turn off MPI support." OFF)
OPTION(BUILD_OSSIM_ID_SUPPORT "Set to ON to build OSSIM GIT ID support into the library.  Use OFF to turn off ID support." ON)
OPTION(BUILD_OSSIM_APPS "Set to ON to build OSSIM applications." ON)
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

# Stores list of libs to link with. Initialized throughout.
set( ossimDependentLibs )

#---
# Find required and optional packages and add their include paths and libraries:
#---

# GEOS - Currently optional until it is actually called.:
find_package( GEOS )
if( GEOS_FOUND )
   include_directories( ${GEOS_INCLUDE_DIR} )
   set( ossimDependentLibs ${ossimDependentLibs} ${GEOS_LIBRARY} )
else( GEOS_FOUND )
   message( FATAL_ERROR "Could not find geos package! Consider installing this as it will soon become a required package." )
endif( GEOS_FOUND )

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

# JPEG - Required package:
find_package( JPEG )
if( JPEG_FOUND )
   include_directories( ${JPEG_INCLUDE_DIR} )
   set( ossimDependentLibs ${ossimDependentLibs} ${JPEG_LIBRARY} )
else( JPEG_FOUND )
   message( FATAL_ERROR "Could not find required jpeg package!" )
endif( JPEG_FOUND )

# OpenThreads - Required:
find_package( OpenThreads )
if( OPENTHREADS_FOUND )
   include_directories( ${OPENTHREADS_INCLUDE_DIR} )
   set( ossimDependentLibs ${ossimDependentLibs} ${OPENTHREADS_LIBRARY} )
else( OPENTHREADS_FOUND )
   message( FATAL_ERROR "Could not find required OpenThreads package!" )
endif( OPENTHREADS_FOUND )

# TIFF - Required:
find_package( TIFF )
if( TIFF_FOUND )
   include_directories( ${TIFF_INCLUDE_DIR} )
   set( ossimDependentLibs ${ossimDependentLibs} ${TIFF_LIBRARY} )
else( TIFF_FOUND )
   message( FATAL_ERROR "Could not find required tiff package!" )
endif( TIFF_FOUND )

# DL - Required on unix:
if( UNIX )
   find_library( DL_LIBRARY dl )
   if ( DL_LIBRARY )
      set( ossimDependentLibs ${ossimDependentLibs} ${DL_LIBRARY} )
   else( DL_LIBRARY )
      # TODO:  Is this required on unix?  If so move to the required section.
      message( FATAL_ERROR "Could not find dl library!" )
   endif( DL_LIBRARY )
endif( UNIX )

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
      include_directories( ${MPI_INCLUDE_DIR} )
      set( ossimDependentLibs ${ossimDependentLibs} ${MPI_LIBRARY} )
      set( OSSIM_HAS_MPI 1 )
   else ( MPI_FOUND )
      message( WARNING "Could not find optional MPI package!" )
   endif ( MPI_FOUND )
endif( BUILD_OSSIM_MPI_SUPPORT )

# ZLIB - Optional:
set( OSSIM_HAS_LIBZ 0 )
find_package( ZLIB )
if ( ZLIB_FOUND )
   include_directories( ${ZLIB_INCLUDE_DIR} )
   set( ossimDependentLibs ${ossimDependentLibs} ${ZLIB_LIBRARY} )
   set( OSSIM_HAS_LIBZ 1 )
else ( ZLIB_FOUND )
   message( WARNING "Could not find optional zlib package!" )
endif ( ZLIB_FOUND )

#---
# Call the OSSIM macros in OssimUtilities.cmake
#---
TODAYS_DATE(OSSIM_BUILD_DATE)
SET(OSSIM_GIT_REVISION_NUMBER "UNKNOWN")
GET_GIT_REVISION()
if ( NOT ${Project_WC_REVISION} EQUAL 0 )
   set( OSSIM_GIT_REVISION_NUMBER ${Project_WC_REVISION} )
endif()

#####################################################################################
# Call the configure files for ossimConfig and ossimVersion setup
#####################################################################################
set(OSSIM_VERSION_NUMBER "\"${OSSIM_VERSION}\"")
set(OSSIM_BUILD_DATE "\"${OSSIM_BUILD_DATE}\"")

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

set(TARGET_COMMON_LIBRARIES ossim ${OPENTHREADS_LIBRARY})

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

###################### OUTPUT GENERAL VARIABLE SETTINGS #######################

MESSAGE( STATUS "OSSIM_GIT_REVISION_NUMBER       = ${OSSIM_GIT_REVISION_NUMBER}" )
MESSAGE( STATUS "FREETYPE_LIBRARY                = ${FREETYPE_LIBRARIES}" )
MESSAGE( STATUS "FREETYPE_INCLUDE                = ${FREETYPE_INCLUDE_DIRS}" )
MESSAGE( STATUS "GEOTIFF_LIBRARY                 = ${GEOTIFF_LIBRARIES}" )
MESSAGE( STATUS "GEOTIFF_INCLUDE                 = ${GEOTIFF_INCLUDE_DIR}" )
MESSAGE( STATUS "GEOTRANS_LIBRARY                = ${GEOTRANS_LIBRARY}" )
MESSAGE( STATUS "GEOTRANS_INCLUDE                = ${GEOTRANS_INCLUDE_DIR}" )
MESSAGE( STATUS "GEOS_LIBRARY                    = ${GEOS_LIBRARY}" )
MESSAGE( STATUS "GEOS_INCLUDE                    = ${GEOS_INCLUDE_DIR}" )
MESSAGE( STATUS "JPEG_LIBRARY                    = ${JPEG_LIBRARIES}" )
MESSAGE( STATUS "JPEG_INCLUDE                    = ${JPEG_INCLUDE_DIR}" )
MESSAGE( STATUS "OSSIM DEPENDENT LIBRARIES       = ${ossimDependentLibs}" )

#-----------------------------------------------------------------------------
# uninstall target
#-----------------------------------------------------------------------------
#OSSIM_ADD_COMMON_MAKE_UNINSTALL()

message("################## end of OSSIM core library setup #########################")

IF(BUILD_OSSIM_APPS)
   add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/apps ${CMAKE_CURRENT_BINARY_DIR}/ossim/apps)
ENDIF()

IF(BUILD_OSSIM_TESTS)
   add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test ${CMAKE_CURRENT_BINARY_DIR}/ossim/test)
ENDIF()


