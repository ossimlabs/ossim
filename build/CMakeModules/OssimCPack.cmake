# This script sets up packaging targets for each "COMPONENT" as specified in INSTALL commands
#
# for each component a CPackConfig-<component>.cmake is generated in the build tree
# and a target is added to call cpack for it (e.g. package_ossim
# A target for generating a package with everything that gets INSTALLED is generated (package_ossim-all)
# A target for making all of the above packages is generated (package_ALL)
#
# package filenames are created on the form <package>-<platform>-<arch>[-<compiler>]-<build_type>[-static].tgz
# ...where compiler optionally set using a cmake gui (OSSIM_CPACK_COMPILER). This script tries to guess compiler version for msvc generators
# ...build_type matches CMAKE_BUILD_TYPE for all generators but the msvc ones

# resolve architecture. The reason i "change" i686 to i386 is that debian packages
# require i386 so this is for the future
IF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i686")
    SET(SYSTEM_ARCH "i386")
ELSE()
    SET(SYSTEM_ARCH ${CMAKE_SYSTEM_PROCESSOR})
ENDIF()

# set a default system name - use CMake setting (Linux|Windows|...)
SET(SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
#message(STATUS "CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}")
#message(STATUS "CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR}")

# for msvc the SYSTEM_NAME is set win32/64 instead of "Windows"
IF(MSVC)
    IF(CMAKE_CL_64)
        SET(SYSTEM_NAME "win64")
    ELSE()
        SET(SYSTEM_NAME "win32")
    ENDIF()
ENDIF()
# Guess the compiler (is this desired for other platforms than windows?)
IF(NOT DEFINED OSSIM_CPACK_COMPILER)
    INCLUDE(OssimDetermineCompiler)
ENDIF()

# expose the compiler setting to the user
SET(OSSIM_CPACK_COMPILER "${OSSIM_COMPILER}" CACHE STRING "This ia short string (vc90, vc80sp1, gcc-4.3, ...) describing your compiler. The string is used for creating package filenames")

IF(OSSIM_CPACK_COMPILER)
  SET(OSSIM_CPACK_SYSTEM_SPEC_STRING ${SYSTEM_NAME}-${SYSTEM_ARCH}-${OSSIM_CPACK_COMPILER})
ELSE()
  SET(OSSIM_CPACK_SYSTEM_SPEC_STRING ${SYSTEM_NAME}-${SYSTEM_ARCH})
ENDIF()


## variables that apply to all packages
SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${OSSIM_VERSION}")

IF(NOT CPACK_GENERATOR)
   # these goes for all platforms. Setting these stops the CPack.cmake script from generating options about other package compression formats (.z .tz, etc.)
   IF(WIN32)
       SET(CPACK_GENERATOR "ZIP")
       SET(CPACK_SOURCE_GENERATOR "ZIP")
   ELSEIF(APPLE)
#       SET(CPACK_GENERATOR "PackageMaker")
#       SET(CPACK_SOURCE_GENERATOR "PackageMaker")
       SET(CPACK_GENERATOR "TGZ")
       SET(CPACK_SOURCE_GENERATOR "TGZ")
   ELSE()
       SET(CPACK_GENERATOR "TGZ")
       SET(CPACK_SOURCE_GENERATOR "TGZ")
   ENDIF()
ENDIF(NOT CPACK_GENERATOR)

# for ms visual studio we use it's internally defined variable to get the configuration (debug,release, ...) 
IF(MSVC_IDE)
    SET(OSSIM_CPACK_CONFIGURATION "$(OutDir)")
    SET(PACKAGE_TARGET_PREFIX "Package ")
ELSE()
    # on un*x an empty CMAKE_BUILD_TYPE means release
    IF(CMAKE_BUILD_TYPE)
        SET(OSSIM_CPACK_CONFIGURATION ${CMAKE_BUILD_TYPE})
    ELSE()
        SET(OSSIM_CPACK_CONFIGURATION "Release")
    ENDIF()
    SET(PACKAGE_TARGET_PREFIX "package_")
ENDIF()

# Get all defined components
GET_CMAKE_PROPERTY(CPACK_COMPONENTS_ALL COMPONENTS)
IF(NOT CPACK_COMPONENTS_ALL)
  # cmake 2.6.0 don't supply the COMPONENTS property.
  # I set it manually to be the packages that can always be packaged
  MESSAGE("When building packages please consider using cmake version 2.6.1 or above")
  SET(CPACK_COMPONENTS_ALL ossim)
ENDIF()

# Create a target that will be used to generate all packages defined below
SET(PACKAGE_ALL_TARGETNAME "${PACKAGE_TARGET_PREFIX}ALL")
ADD_CUSTOM_TARGET(${PACKAGE_ALL_TARGETNAME})

MACRO(GENERATE_PACKAGING_TARGET package_name)
    SET(CPACK_PACKAGE_NAME ${package_name})

    # the doc packages don't need a system-arch specification
    IF(${package} MATCHES -doc)
        SET(OSSIM_PACKAGE_FILE_NAME ${package_name}-${OSSIM_VERSION})
    ELSE()
        SET(OSSIM_PACKAGE_FILE_NAME ${package_name}-${OSSIM_VERSION}-${OSSIM_CPACK_SYSTEM_SPEC_STRING}-${OSSIM_CPACK_CONFIGURATION})
        IF(NOT BUILD_SHARED_LIBS)
            SET(OSSIM_PACKAGE_FILE_NAME ${OSSIM_PACKAGE_FILE_NAME}-static)
        ENDIF()
    ENDIF()

    CONFIGURE_FILE("${${PROJECT_NAME}_SOURCE_DIR}/CMakeModules/OssimCPackConfig.cmake.in" "${${PROJECT_NAME}_BINARY_DIR}/CPackConfig-${package_name}.cmake" IMMEDIATE)

    SET(PACKAGE_TARGETNAME "${PACKAGE_TARGET_PREFIX}${package_name}")

    # This is naive and will probably need fixing eventually
    IF(MSVC)
        SET(MOVE_COMMAND "move")
    ELSE()
        SET(MOVE_COMMAND "mv")
    ENDIF()
    
    IF(NOT ACHIVE_EXT)
       # Set in and out archive filenames. Windows = zip, others = tgz
       IF(WIN32)
          SET(ARCHIVE_EXT "zip")
       ELSEIF(APPLE)
#           SET(ARCHIVE_EXT "dmg")
           SET(ARCHIVE_EXT "tar.gz")
      ELSE()
           SET(ARCHIVE_EXT "tar.gz")
       ENDIF()
    ENDIF(NOT ACHIVE_EXT)

    # Create a target that creates the current package
    # and rename the package to give it proper filename
    ADD_CUSTOM_TARGET(${PACKAGE_TARGETNAME})
    ADD_CUSTOM_COMMAND(TARGET ${PACKAGE_TARGETNAME}
        COMMAND ${CMAKE_CPACK_COMMAND} -C ${OSSIM_CPACK_CONFIGURATION} --config ${${PROJECT_NAME}_BINARY_DIR}/CPackConfig-${package_name}.cmake
        COMMAND "${MOVE_COMMAND}" "${CPACK_PACKAGE_FILE_NAME}.${ARCHIVE_EXT}" "${OSSIM_PACKAGE_FILE_NAME}.${ARCHIVE_EXT}"
        COMMAND ${CMAKE_COMMAND} -E echo "renamed ${CPACK_PACKAGE_FILE_NAME}.${ARCHIVE_EXT} -> ${OSSIM_PACKAGE_FILE_NAME}.${ARCHIVE_EXT}"
        COMMENT "Run CPack packaging for ${package_name}..."
    )
    # Add the exact same custom command to the all package generating target. 
    # I can't use add_dependencies to do this because it would allow parallel building of packages so am going brute here
    ADD_CUSTOM_COMMAND(TARGET ${PACKAGE_ALL_TARGETNAME}
        COMMAND ${CMAKE_CPACK_COMMAND} -C ${OSSIM_CPACK_CONFIGURATION} --config ${${PROJECT_NAME}_BINARY_DIR}/CPackConfig-${package_name}.cmake
        COMMAND "${MOVE_COMMAND}" "${CPACK_PACKAGE_FILE_NAME}.${ARCHIVE_EXT}" "${OSSIM_PACKAGE_FILE_NAME}.${ARCHIVE_EXT}"
        COMMAND ${CMAKE_COMMAND} -E echo "renamed ${CPACK_PACKAGE_FILE_NAME}.${ARCHIVE_EXT} -> ${OSSIM_PACKAGE_FILE_NAME}.${ARCHIVE_EXT}"
    )
ENDMACRO(GENERATE_PACKAGING_TARGET)

# Create configs and targets for a package including all components
SET(OSSIM_CPACK_COMPONENT ALL)
GENERATE_PACKAGING_TARGET(ossim-all)

# Create configs and targets for each component
FOREACH(package ${CPACK_COMPONENTS_ALL})
    SET(OSSIM_CPACK_COMPONENT ${package})
    GENERATE_PACKAGING_TARGET(${package})
ENDFOREACH()
