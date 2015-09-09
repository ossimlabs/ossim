# - Extract information from a GIT working copy
# The module defines the following variables:
#    GIT_EXECUTABLE - path to git command line client
#    GIT_FOUND - true if the command line client was found
#    GIT_VERSION_STRING - the version of git found (since CMake 2.8.8)
#
# The minimum required version of Git can be specified using the
# standard syntax, e.g. FIND_PACKAGE(Git 1.4)
#
# If the command line client executable is found two macros are defined:
#  GIT_WC_INFO(<dir> <var-prefix>)
#  GIT_WC_LOG(<dir> <var-prefix>)
# GIT_WC_INFO extracts information of a GIT working copy at
# a given location. This macro defines the following variables:
#  <var-prefix>_WC_URL - url of the repository (at <dir>)
#  <var-prefix>_WC_ROOT - root url of the repository
#  <var-prefix>_WC_REVISION - current revision
#  <var-prefix>_WC_LAST_CHANGED_AUTHOR - author of last commit
#  <var-prefix>_WC_LAST_CHANGED_DATE - date of last commit
#  <var-prefix>_WC_LAST_CHANGED_REV - revision of last commit
#  <var-prefix>_WC_INFO - output of command `svn info <dir>'
# GIT_WC_LOG retrieves the log message of the base revision of a
# GIT working copy at a given location. This macro defines the
# variable:
#  <var-prefix>_LAST_CHANGED_LOG - last log of base revision
# Example usage:
#  FIND_PACKAGE(GIT)
#  IF(GIT_FOUND)
#    GIT_WC_INFO(${PROJECT_SOURCE_DIR} Project)
#    MESSAGE("Current revision is ${Project_WC_REVISION}")
#    GIT_WC_LOG(${PROJECT_SOURCE_DIR} Project)
#    MESSAGE("Last changed log is ${Project_LAST_CHANGED_LOG}")
#  ENDIF(GIT_FOUND)
#
# History (yyyymmdd):
# 20121002 - Modified to "continue on" if svn is not found. (drb) 
#
#=============================================================================
# Copyright 2010 Kitware, Inc.
# Copyright 2012 Rolf Eike Beer <eike@sf-mail.de>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# Look for 'git' or 'eg' (easy git)
#
set(git_names git eg)

# Prefer .cmd variants on Windows unless running in a Makefile
# in the MSYS shell.
#
if(WIN32)
  if(NOT CMAKE_GENERATOR MATCHES "MSYS")
    set(git_names git.cmd git eg.cmd eg)
    # GitHub search path for Windows
    set(github_path "$ENV{LOCALAPPDATA}/Github/PortableGit*/bin")
    file(GLOB github_path "${github_path}")
  endif()
endif()

find_program(GIT_EXECUTABLE
  NAMES ${git_names}
  PATHS ${github_path}
  PATH_SUFFIXES Git/cmd Git/bin
  DOC "git command line client"
  )
mark_as_advanced(GIT_EXECUTABLE)

IF(GIT_EXECUTABLE)
  execute_process(COMMAND ${GIT_EXECUTABLE} --version
                  OUTPUT_VARIABLE git_version
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (git_version MATCHES "^git version [0-9]")
    string(REPLACE "git version " "" GIT_VERSION_STRING "${git_version}")
  endif()
  unset(git_version)

  MACRO(GIT_WC_INFO dir prefix)
    # the GIT commands should be executed with the C locale, otherwise
    # the message (which are parsed) may be translated, Alex
    # SET(GIT_SAVED_LC_ALL "$ENV{LC_ALL}")
    # SET(ENV{LC_ALL} C)

    EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} rev-list HEAD --count
      OUTPUT_VARIABLE ${prefix}_WC_REVISION
      ERROR_VARIABLE git_info_error
      RESULT_VARIABLE git_info_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(NOT ${git_info_result} EQUAL 0)
      MESSAGE(WARNING "Command \"${GIT_EXECUTABLE} rev-list HEAD --count\" failed with output:\n${git_info_error}")
    ELSE(NOT ${git_info_result} EQUAL 0)
	set(${prefix}_WC_INFO ${git_info_result})
    ENDIF(NOT ${git_info_result} EQUAL 0)

    # restore the previous LC_ALL
    # SET(ENV{LC_ALL} ${GIT_SAVED_LC_ALL})

  ENDMACRO(GIT_WC_INFO)

ENDIF(GIT_EXECUTABLE)

# Handle the QUIETLY and REQUIRED arguments and set GIT_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( GIT DEFAULT_MSG
                                   GIT_EXECUTABLE 
                                   GIT_VERSION_STRING )

if( GIT_FOUND )
   if( NOT GIT_FIND_QUIETLY )
     message( STATUS "GIT_VERSION_STRING              = ${GIT_VERSION_STRING}" )
   endif( NOT GIT_FIND_QUIETLY )
else( GIT_FOUND )
   if( NOT GIT_FIND_QUIETLY )
      message( WARNING "Could not find GIT" )
   endif( NOT GIT_FIND_QUIETLY )
endif( GIT_FOUND )


