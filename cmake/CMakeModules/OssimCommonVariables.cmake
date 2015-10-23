
##################################################################################
# This currently sets up the options for the WARNING FLAGS for the compiler we are generating for.
# Currently only have gnu
##################################################################################
MACRO(OSSIM_ADD_COMMON_LIBRARY_FLAGS)
   OPTION(OSSIM_COMPILE_WITH_FULL_WARNING "OSSIM developers : Compilation with FULL warning (use only for ossim developers)." OFF)
   MARK_AS_ADVANCED(OSSIM_COMPILE_WITH_FULL_WARNING)
   
   IF(OSSIM_COMPILE_WITH_FULL_WARNING)
     IF(CMAKE_COMPILER_IS_GNUCXX)
       SET(OSSIM_COMMON_COMPILER_FLAGS "${OSSIM_COMMON_COMPILER_FLAGS} -Wall -Wunused  -Wunused-function  -Wunused-label  -Wunused-parameter -Wunused-value -Wunused-variable -Wuninitialized -Wsign-compare -Wparentheses -Wunknown-pragmas -Wswitch" CACHE STRING "List of compilation parameters.")
     ENDIF(CMAKE_COMPILER_IS_GNUCXX)
   ENDIF(OSSIM_COMPILE_WITH_FULL_WARNING)

   IF(WIN32)
      #---
      # This option is to enable the /MP to compile multiple source files by using 
      # multiple processes.
      #---
      OPTION(WIN32_USE_MP "Set to ON to build OSSIM with the /MP option (Visual Studio 2005 and above)." OFF)
      MARK_AS_ADVANCED(WIN32_USE_MP)
      IF(WIN32_USE_MP)
         SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
      ENDIF(WIN32_USE_MP)
     
      set(OSSIM_COMMON_COMPILER_FLAGS "${OSSIM_COMMON_COMPILER_FLAGS} -DNOMINMAX -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE") 
 
      set(DEBUG_BUILD OFF)
      IF(CMAKE_BUILD_TYPE)
         string ( COMPARE EQUAL ${CMAKE_BUILD_TYPE} "Debug" DEBUG_BUILD )
      ENDIF(CMAKE_BUILD_TYPE)
     
      ###
      # Currently must set /FORCE:MULTIPLE for Visual Studio 2010. 29 October 2010 - drb
      ###
  
      IF(MSVC)
         message("MSVC_VERSION: ${MSVC_VERSION}")

         if( (${MSVC_VERSION} EQUAL 1600) OR (${MSVC_VERSION} EQUAL 1700) )
            if (DEBUG_BUILD)
               SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBCMTD /NODEFAULTLIB:MSVCRT /FORCE:MULTIPLE /MANIFEST:NO")
               SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMTD /FORCE:MULTIPLE /MANIFEST:NO")

            else ( )
               SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBCMTD /FORCE:MULTIPLE /MANIFEST:NO")
               SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMTD /FORCE:MULTIPLE /MANIFEST:NO")
            endif (DEBUG_BUILD)

            SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMTD /FORCE:MULTIPLE")
         else( )
            SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBCMTD")
         endif( (${MSVC_VERSION} EQUAL 1600) OR (${MSVC_VERSION} EQUAL 1700) )
      ENDIF(MSVC)
   ENDIF(WIN32)
   
   OPTION(OSSIM_ADD_FPIC "Compilation with FPIC flag if static library.  The default is on since we have plugins that need to be shared." ON)
   MARK_AS_ADVANCED(OSSIM_ADD_FPIC)
   IF(OSSIM_ADD_FPIC)
       IF(UNIX AND NOT BUILD_SHARED_LIBS)
          STRING(REGEX MATCH "fPIC" REG_MATCHED "${OSSIM_COMMON_COMPILER_FLAGS}")
          if(NOT REG_MATCHED)
             set(OSSIM_COMMON_COMPILER_FLAGS "${OSSIM_COMMON_COMPILER_FLAGS} -fPIC")
          endif(NOT REG_MATCHED)
       ENDIF(UNIX AND NOT BUILD_SHARED_LIBS)
   ENDIF(OSSIM_ADD_FPIC)

   MARK_AS_ADVANCED(OSSIM_COMMON_COMPILER_FLAGS)
ENDMACRO(OSSIM_ADD_COMMON_LIBRARY_FLAGS)

MACRO(OSSIM_ADD_COMMON_SETTINGS)
   ###################################################################################
   # Set defaults for Universal Binaries. We want 32-bit Intel/PPC on 10.4
   # and 32/64-bit Intel/PPC on >= 10.5. Anything <= 10.3 doesn't support.
   # These are just defaults/recommendations, but how we want to build
   # out of the box. But the user needs to be able to change these options.
   # So we must only set the values the first time CMake is run, or we
   # will overwrite any changes the user sets.
   # FORCE is used because the options are not reflected in the UI otherwise.
   # Seems like a good place to add version specific compiler flags too.
   ###################################################################################
   IF(APPLE)
        SET(TEMP_CMAKE_OSX_ARCHITECTURES "")
        SET(CMAKE_OSX_SYSROOT "${CMAKE_OSX_SYSROOT}")
        # This is really fragile, but CMake doesn't provide the OS system
        # version information we need. (Darwin versions can be changed
        # independently of OS X versions.)
        # It does look like CMake handles the CMAKE_OSX_SYSROOT automatically.
        IF(EXISTS /Developer/SDKs/MacOSX10.6.sdk)
            SET(TEMP_CMAKE_OSX_ARCHITECTURES "i386;x86_64")
            IF(NOT ("${CMAKE_CXX_FLAGS}" MATCHES "mmacosx-version-min"))
               SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.5 -ftree-vectorize -fvisibility-inlines-hidden" CACHE STRING "Flags used by the compiler during all build types.")
            ENDIF()
        ELSEIF(EXISTS /Developer/SDKs/MacOSX10.5.sdk)
            # 64-bit compiles are not supported with Carbon. We should enable 
            SET(TEMP_CMAKE_OSX_ARCHITECTURES "i386;x86_64")
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.5 -ftree-vectorize -fvisibility-inlines-hidden" CACHE STRING "Flags used by the compiler during all build types.")
        ELSEIF(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)
            SET(TEMP_CMAKE_OSX_ARCHITECTURES "i386;ppc")
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.4 -ftree-vectorize -fvisibility-inlines-hidden" CACHE STRING "Flags used by the compiler during all build types.")
        ELSE()
            # No Universal Binary support
            # Should break down further to set the -mmacosx-version-min,
            # but the SDK detection is too unreliable here.
        ENDIF()
        IF(NOT CMAKE_OSX_ARCHITECTURES)
           SET(CMAKE_OSX_ARCHITECTURES "${TEMP_CMAKE_OSX_ARCHITECTURES}" CACHE STRING "Build architectures for OSX" FORCE)
        ENDIF()
        OPTION(OSSIM_BUILD_APPLICATION_BUNDLES "Enable the building of applications and examples as OSX Bundles" OFF)
        
       MARK_AS_ADVANCED(CMAKE_OSX_ARCHITECTURES)
       MARK_AS_ADVANCED(CMAKE_CXX_FLAGS)
       MARK_AS_ADVANCED(CMAKE_OSX_SYSROOT)
       MARK_AS_ADVANCED(OSSIM_BUILD_APPLICATION_BUNDLES)
   ENDIF(APPLE)

  SET(MAKE_APPENDS_BUILD_TYPE "NO")
  IF(CMAKE_GENERATOR)
     STRING(TOUPPER ${CMAKE_GENERATOR} CMAKE_GENERATOR_TEST_UPPER)
     STRING(COMPARE EQUAL "${CMAKE_GENERATOR_TEST_UPPER}" "XCODE" CMAKE_GENERATOR_TEST)
     IF(CMAKE_GENERATOR_TEST)
          SET(MAKE_APPENDS_BUILD_TYPE "YES")
     ELSE()
          STRING(COMPARE NOTEQUAL "." "${CMAKE_CFG_INTDIR}" CMAKE_GENERATOR_TEST)
          IF(CMAKE_GENERATOR_TEST)
             SET(MAKE_APPENDS_BUILD_TYPE "YES")
          ENDIF()
     ENDIF()
  ENDIF(CMAKE_GENERATOR)
   OSSIM_ADD_COMMON_LIBRARY_FLAGS()

   # Dynamic vs Static Linking
   OPTION(BUILD_SHARED_LIBS "Set to ON to build OSSIM for dynamic linking.  Use OFF for static." ON)
   OPTION(BUILD_OSSIM_FRAMEWORKS "Set to ON to build OSSIM for framework if BUILD_SHARED_LIBS is on.  Use OFF for dylib if BUILD_SHARED_LIBS is on." ON)
   IF(BUILD_SHARED_LIBS)
       SET(OSSIM_USER_DEFINED_DYNAMIC_OR_STATIC "SHARED")
   ELSE ()
       SET(OSSIM_USER_DEFINED_DYNAMIC_OR_STATIC "STATIC")
   ENDIF()

#   IF(NOT OSSIM_LIBRARY_BUILD_OUTPUT_DIRECTORY)  
#      SET(OSSIM_LIBRARY_BUILD_OUTPUT_DIRECTORY ${${PROJECT_NAME}_SOURCE_DIR}/lib)
#   ENDIF(NOT OSSIM_LIBRARY_BUILD_OUTPUT_DIRECTORY)  
#   IF(NOT OSSIM_RUNTIME_BUILD_OUTPUT_DIRECTORY)  
#      SET(OSSIM_RUNTIME_BUILD_OUTPUT_DIRECTORY ${${PROJECT_NAME}_SOURCE_DIR}/bin)
#   ENDIF(NOT OSSIM_RUNTIME_BUILD_OUTPUT_DIRECTORY)  

   IF(MAKE_APPENDS_BUILD_TYPE)
      SET(BUILD_FRAMEWORK_DIR "")
      SET(BUILD_RUNTIME_DIR   "bin")
      SET(BUILD_LIBRARY_DIR   "lib")
      SET(BUILD_ARCHIVE_DIR   "lib")
      SET(BUILD_INCLUDE_DIR   "include")
   ELSE()
      IF(NOT DEFINED BUILD_FRAMEWORK_DIR)
         SET(BUILD_FRAMEWORK_DIR "")
      ENDIF()
      IF(NOT DEFINED BUILD_RUNTIME_DIR)
         SET(BUILD_RUNTIME_DIR   "bin")
      ENDIF()
      IF(NOT DEFINED BUILD_LIBRARY_DIR)  
         SET(BUILD_LIBRARY_DIR   "lib")
      ENDIF()
      IF(NOT DEFINED BUILD_ARCHIVE_DIR)
         SET(BUILD_ARCHIVE_DIR   "lib")
      ENDIF()
      IF(NOT DEFINED BUILD_INCLUDE_DIR)
         SET(BUILD_INCLUDE_DIR   "include")
      ENDIF()
   ENDIF()

   SET(INSTALL_FRAMEWORK_DIR "Frameworks")
   SET(INSTALL_RUNTIME_DIR   "bin")
   SET(INSTALL_INCLUDE_DIR   "include")

   # Libraries and archives go to lib or lib64.
   get_property(LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)       
   if(LIB64)
      set(LIBSUFFIX 64)
   else()
      set(LIBSUFFIX "")
   endif()   
   set(INSTALL_LIBRARY_DIR lib${LIBSUFFIX} CACHE PATH "Installation directory for libraries")
   set(INSTALL_ARCHIVE_DIR lib${LIBSUFFIX} CACHE PATH "Installation directory for archive")
   mark_as_advanced(LIBSUFFIX)
   mark_as_advanced(INSTALL_LIBRARY_DIR)
   mark_as_advanced(INSTALL_ARCHIVE_DIR)
   
   IF(WIN32)
       IF(NOT DEFINED CMAKE_DEBUG_POSTFIX)
          SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Debug variable used to add the postfix to dll's and exe's.  Defaults to 'd' on WIN32 builds and empty on all other platforms" FORCE)
       ENDIF()
   ENDIF()
   IF(NOT CMAKE_BUILD_TYPE)
     SET(CMAKE_BUILD_TYPE Release)
   ENDIF(NOT CMAKE_BUILD_TYPE)

   # force some variables that could be defined in the command line to be written to cache
   SET( CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE PATH
     "Where to install ${PROJECT_NAME}" FORCE )
   SET( CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
     "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE )
   SET( CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" CACHE PATH
     "Path to custom CMake Modules" FORCE )
   SET( INSTALL_DOC "${INSTALL_DOC}" CACHE BOOL
     "Set to OFF to skip build/install Documentation" FORCE )

   IF(NOT OSSIM_DEV_HOME)
      SET(OSSIM_DEV_HOME "$ENV{OSSIM_DEV_HOME}")
      GET_FILENAME_COMPONENT(OSSIM_DEV_HOME "${OSSIM_DEV_HOME}" ABSOLUTE)
   ENDIF(NOT OSSIM_DEV_HOME)
   IF(NOT CMAKE_INSTALL_PREFIX)
      SET(CMAKE_INSTALL_PREFIX "$ENV{OSSIM_INSTALL_PREFIX}")
       GET_FILENAME_COMPONENT(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" ABSOLUTE)   
   ENDIF(NOT CMAKE_INSTALL_PREFIX)
   IF(NOT OSSIM_DEPENDENCIES)
      SET(OSSIM_DEPENDENCIES "$ENV{OSSIM_DEPENDENCIES}")
      GET_FILENAME_COMPONENT(OSSIM_DEPENDENCIES "${OSSIM_DEPENDENCIES}" ABSOLUTE)   
   ENDIF(NOT OSSIM_DEPENDENCIES)
 
   #################################### ADd some common options for all modules to use ###################################
   OPTION(BUILD_OSSIM_VIDEO "Set to ON to build the video decoding library.  Use OFF to skip this module." OFF)
   OPTION(BUILD_OSSIM_PLANET "Set to ON to build the 3-D visualization module.  Use OFF to skip this module." OFF)
   OPTION(BUILD_OSSIM_GUI "Set to ON to build the new ossimGui library and geocell application." ON)
   OPTION(BUILD_MRSID_PLUGIN "Set to ON to build the MrSID plugin library." ON)   
   OPTION(BUILD_KAKADU_PLUGIN "Set to ON to build the Kakadu plugin library." ON)
   OPTION(BUILD_PDAL_PLUGIN "Set to ON to build the PDAL plugin library." ON)
   OPTION(BUILD_GDAL_PLUGIN "Set to ON to build the GDAL plugin library." ON)
   OPTION(BUILD_OMS "Set to ON to build the oms api library." ON)
   OPTION(BUILD_WMS "Set to ON to build the wms api library." ON)


ENDMACRO(OSSIM_ADD_COMMON_SETTINGS)


OSSIM_ADD_COMMON_SETTINGS()

