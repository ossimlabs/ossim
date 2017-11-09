# Locate ffmpeg
# This module defines
# FFMPEG_LIBRARIES
# FFMPEG_FOUND, if false, do not try to link to ffmpeg
# FFMPEG_INCLUDE_DIR, where to find the headers
#
# $FFMPEG_DIR is an environment variable that would
# correspond to the ./configure --prefix=$FFMPEG_DIR
#
# Created by Robert Osfield.


#In ffmpeg code, old version use "#include <header.h>" and newer use "#include <libname/header.h>"
#In OSG ffmpeg plugin, we use "#include <header.h>" for compatibility with old version of ffmpeg

#We have to search the path which contain the header.h (usefull for old version)
#and search the path which contain the libname/header.h (usefull for new version)

#Then we need to include ${FFMPEG_libname_INCLUDE_DIRS} (in old version case, use by ffmpeg header and osg plugin code)
#                                                       (in new version case, use by ffmpeg header) 
#and ${FFMPEG_libname_INCLUDE_DIRS/libname}             (in new version case, use by osg plugin code)


# Macro to find header and lib directories
# example: FFMPEG_FIND(AVFORMAT avformat avformat.h)
MACRO(FFMPEG_FIND varname shortname headername)
    # old version of ffmpeg put header in $prefix/include/[ffmpeg]
    # so try to find header in include directory

    FIND_PATH(FFMPEG_${varname}_INCLUDE_DIRS lib${shortname}/${headername}
        PATHS
        ${FFMPEG_ROOT}/include
        $ENV{FFMPEG_DIR}/include
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/include
        /usr/include
        /sw/include # Fink
        /opt/local/include # DarwinPorts
        /opt/csw/include # Blastwave
        /opt/include
        /usr/freeware/include
        PATH_SUFFIXES ffmpeg
        DOC "Location of FFMPEG Headers"
    )

    if( FFMPEG_${varname}_INCLUDE_DIRS-NOTFOUND )
       FIND_PATH(FFMPEG_${varname}_INCLUDE_DIRS ${headername}
          PATHS
          ${FFMPEG_ROOT}/include
          $ENV{FFMPEG_DIR}/include
          ~/Library/Frameworks
          /Library/Frameworks
          /usr/local/include
          /usr/include
          /sw/include # Fink
          /opt/local/include # DarwinPorts
          /opt/csw/include # Blastwave
          /opt/include
          /usr/freeware/include
          PATH_SUFFIXES ffmpeg
          DOC "Location of FFMPEG Headers"
       )
    endif ( FFMPEG_${varname}_INCLUDE_DIRS-NOTFOUND )

    FIND_LIBRARY(FFMPEG_${varname}_LIBRARIES
        NAMES ${shortname}
        PATHS
        ${FFMPEG_ROOT}/lib
        $ENV{FFMPEG_DIR}/lib
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/local/lib64
        /usr/lib
        /usr/lib64
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        /usr/freeware/lib64
        DOC "Location of FFMPEG Libraries"
    )

    IF (FFMPEG_${varname}_LIBRARIES AND FFMPEG_${varname}_INCLUDE_DIRS)
        SET(FFMPEG_${varname}_FOUND 1)
    ENDIF(FFMPEG_${varname}_LIBRARIES AND FFMPEG_${varname}_INCLUDE_DIRS)

ENDMACRO(FFMPEG_FIND)

SET(FFMPEG_ROOT "$ENV{FFMPEG_DIR}" CACHE PATH "Location of FFMPEG")

# find stdint.h
IF(WIN32)

    FIND_PATH(FFMPEG_STDINT_INCLUDE_DIR stdint.h
        PATHS
        ${FFMPEG_ROOT}/include
        $ENV{FFMPEG_DIR}/include
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/include
        /usr/include
        /sw/include # Fink
        /opt/local/include # DarwinPorts
        /opt/csw/include # Blastwave
        /opt/include
        /usr/freeware/include
        PATH_SUFFIXES ffmpeg
        DOC "Location of FFMPEG stdint.h Header"
    )

    IF (FFMPEG_STDINT_INCLUDE_DIR)
        SET(STDINT_OK TRUE)
    ENDIF()

ELSE()

    SET(STDINT_OK TRUE)

ENDIF()

FFMPEG_FIND(LIBAVFORMAT avformat avformat.h)
FFMPEG_FIND(LIBAVDEVICE avdevice avdevice.h)
FFMPEG_FIND(LIBAVCODEC  avcodec  avcodec.h)
FFMPEG_FIND(LIBAVUTIL   avutil   avutil.h)
# FFMPEG_FIND(LIBAVCORE   avcore   avcore.h)
FFMPEG_FIND(LIBSWSCALE  swscale  swscale.h)  
FFMPEG_FIND(LIBSWRESAMPLE  swresample swresample.h)  
FFMPEG_FIND(LIBAVFILTER avfilter avfilter.h)

SET(FFMPEG_FOUND "NO")
# Note we don't check FFMPEG_LIBSWSCALE_FOUND here, it's optional.
IF   (FFMPEG_LIBAVFORMAT_FOUND AND FFMPEG_LIBAVDEVICE_FOUND AND FFMPEG_LIBAVCODEC_FOUND AND FFMPEG_LIBAVUTIL_FOUND AND STDINT_OK)

    SET(FFMPEG_FOUND "YES")

    SET( FFMPEG_INCLUDE_DIRS
         ${FFMPEG_LIBAVFORMAT_INCLUDE_DIRS}
         ${FFMPEG_LIBAVDEVICE_INCLUDE_DIRS}
         ${FFMPEG_LIBAVCODEC_INCLUDE_DIRS}
         ${FFMPEG_LIBAVUTIL_INCLUDE_DIRS}
         ${FFMPEG_LIBSWSCALE_INCLUDE_DIRS}
         ${FFMPEG_LIBAVFILTER_INCLUDE_DIRS}
         ${FFMPEG_LIBSWRESAMPLE_INCLUDE_DIRS} )

    SET(FFMPEG_LIBRARY_DIRS ${FFMPEG_LIBAVFORMAT_LIBRARY_DIRS})

    # Note we don't add FFMPEG_LIBSWSCALE_LIBRARIES here, it will be added if found later.
    SET(FFMPEG_LIBRARIES
        ${FFMPEG_LIBAVCODEC_LIBRARIES}
        ${FFMPEG_LIBAVFORMAT_LIBRARIES}
        ${FFMPEG_LIBAVDEVICE_LIBRARIES}
        ${FFMPEG_LIBAVUTIL_LIBRARIES}
        ${FFMPEG_LIBSWSCALE_LIBRARIES})
        
    IF(FFMPEG_LIBAVFILTER_LIBRARIES)
      SET(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} ${FFMPEG_LIBAVFILTER_LIBRARIES})
    ENDIF()
    IF(FFMPEG_LIBSWRESAMPLE_LIBRARIES)
      SET(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} ${FFMPEG_LIBSWRESAMPLE_LIBRARIES})
    ENDIF()

    IF (FFMPEG_LIBAVCORE_FOUND)
        SET(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} ${FFMPEG_LIBAVCORE_LIBRARIES} )
        SET(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS}
              ${FFMPEG_LIBAVCORE_INCLUDE_DIRS} ${FFMPEG_LIBAVCORE_INCLUDE_DIRS}/libavformat
           )
    ENDIF()
   
#---
# This function sets FFMPEG_FOUND if variables are valid.
#--- 
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( FFMPEG DEFAULT_MSG 
                                   FFMPEG_LIBRARIES
                                   FFMPEG_INCLUDE_DIRS )


#  use aPPEND instead of set.  Seems to work better
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_CONSTANT_MACROS")

ELSE ()
   MESSAGE(STATUS "Could not find FFMPEG")
ENDIF()

if( NOT FFMPEG_FIND_QUIETLY )
   message( STATUS "FFMPEG_INCLUDE_DIRS = ${FFMPEG_INCLUDE_DIRS}" )
   message( STATUS "FFMPEG_LIBRARIES = ${FFMPEG_LIBRARIES}" )
endif( NOT FFMPEG_FIND_QUIETLY )
