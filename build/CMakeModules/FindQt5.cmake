# - Find Qt5
# This module can be used to find Qt5.
# The most important issues are that Qt5 pkgconfig files are installed with PKG_CONFIG_PATH properly
# set, and that Qt5 qmake is available via the system path.
# This module defines a number of key variables and macros.
#
#  Below is a detailed list of variables that FindQt5.cmake sets.
#  QT_FOUND                     If false, don't try to use Qt.
#  QT5_FOUND                    If false, don't try to use Qt5.
#
#  QT_VERSION_MAJOR             The major version of Qt found.
#  QT_VERSION_MINOR             The minor version of Qt found.
#  QT_VERSION_PATCH             The patch version of Qt found.
#
#  QT_BINARY_DIR                Path to "bin" of Qt4
#  QT_DOC_DIR                   Path to "doc" of Qt4
#
#  QT_QTCORE_FOUND              True if QtCore was found.
#  QT_QTGUI_FOUND               True if QtGui was found.
#  QT_QTDBUS_FOUND              True if QtDBus was found.
#  QT_QTNETWORK_FOUND           True if QtNetwork was found.
#  QT_QTTEST_FOUND              True if QtTest was found.
#  QT_QTWIDGETS_FOUND           True if QtWidgets was found.
#  QT_QTXML_FOUND               True if QtXml was found.
#
#  QT_INCLUDES                  List of paths to all include directories of Qt5.
#  QT_INCLUDE_DIR               Path to "include" of Qt4
#  QT_QTCORE_INCLUDE_DIR        Path to "include/QtCore"
#  QT_QTDBUS_INCLUDE_DIR        Path to "include/QtDBus"
#  QT_QTGUI_INCLUDE_DIR         Path to "include/QtGui"
#  QT_QTNETWORK_INCLUDE_DIR     Path to "include/QtNetwork"
#  QT_QTTEST_INCLUDE_DIR        Path to "include/QtTest"
#  QT_QTWIDGETS_INCLUDE_DIR     Path to "include/QtWidgets"
#  QT_QTXML_INCLUDE_DIR         Path to "include/QtXml"
#
#  QT_LIBRARIES                 List of paths to all libraries of Qt5.
#  QT_LIBRARY_DIR               Path to "lib" of Qt4
#  QT_QTCORE_LIBRARY            The QtCore library
#  QT_QTDBUS_LIBRARY            The QtDBus library
#  QT_QTGUI_LIBRARY             The QtGui library
#  QT_QTNETWORK_LIBRARY         The QtNetwork library
#  QT_QTTEST_LIBRARY            The QtTest library
#  QT_QTWIDGETS_LIBRARY         The QtWidgets library
#  QT_QTXML_LIBRARY             The QtXml library
#
# also defined, but NOT for general use are
#  QT_MOC_EXECUTABLE            Where to find the moc tool
#  QT_CONFIG_FLAGS              Flags used when building Qt

# Copyright (C) 2001-2009 Kitware, Inc.
# Copyright (C) 2011 Collabora Ltd. <http://www.collabora.co.uk/>
# Copyright (C) 2011 Nokia Corporation
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF(QT_INCLUDES AND QT_LIBRARIES AND QT_MAJOR_VERSION MATCHES 5)
  # Already in cache, be silent
  SET(QT_FOUND TRUE)
  SET(QT5_FOUND TRUE)
  RETURN()
ENDIF(QT_INCLUDES AND QT_LIBRARIES AND QT_MAJOR_VERSION MATCHES 5)

IF(NOT QT_QMAKE_EXECUTABLE)
  FIND_PROGRAM(QT_QMAKE_EXECUTABLE_FINDQT NAMES qmake qmake5 qmake-qt5
               PATHS "${QT_SEARCH_PATH}/bin" "$ENV{QTDIR}/bin")
  SET(QT_QMAKE_EXECUTABLE ${QT_QMAKE_EXECUTABLE_FINDQT} CACHE PATH "Qt qmake program.")
ENDIF(NOT QT_QMAKE_EXECUTABLE)

EXEC_PROGRAM(${QT_QMAKE_EXECUTABLE} ARGS "-query QT_VERSION" OUTPUT_VARIABLE QTVERSION)
IF(NOT QTVERSION MATCHES "5.*")
  SET(QT_FOUND FALSE)
  SET(QT5_FOUND FALSE)
  IF(Qt5_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "CMake was unable to find Qt5, put qmake in your path or set QTDIR/QT_QMAKE_EXECUTABLE.")
  ENDIF(Qt5_FIND_REQUIRED)
  RETURN()
ENDIF(NOT QTVERSION MATCHES "5.*")

# FIND_PACKAGE(PkgConfig REQUIRED)

IF(NOT Qt5_FIND_COMPONENTS)
  SET(_COMPONENTS QtCore QtDBus QtGui QtNetwork QtTest QtWidgets QtXml)
ELSE(NOT Qt5_FIND_COMPONENTS)
  SET(_COMPONENTS ${Qt5_FIND_COMPONENTS})
ENDIF(NOT Qt5_FIND_COMPONENTS)

FOREACH(_COMPONENT ${_COMPONENTS})
  STRING(TOUPPER ${_COMPONENT} _COMPONENT_UPPER)
  IF(NOT QT_${_COMPONENT_UPPER}_FOUND)
    IF(Qt5_FIND_REQUIRED)
      PKG_CHECK_MODULES(PC_${_COMPONENT} REQUIRED ${_COMPONENT}>=${QT_MIN_VERSION})
    ELSE(Qt5_FIND_REQUIRED)
      PKG_CHECK_MODULES(PC_${_COMPONENT} QUIET ${_COMPONENT}>=${QT_MIN_VERSION})
    ENDIF(Qt5_FIND_REQUIRED)

    SET(QT_${_COMPONENT_UPPER}_INCLUDE_DIR ${PC_${_COMPONENT}_INCLUDE_DIRS})

    FIND_LIBRARY(QT_${_COMPONENT_UPPER}_LIBRARY
                 NAMES ${_COMPONENT}
                 HINTS
                 ${PC_${_COMPONENT}_LIBDIR})

    SET(QT_${_COMPONENT_UPPER}_FOUND ${PC_${_COMPONENT}_FOUND})

    #MESSAGE(STATUS "COMPONENT ${_COMPONENT_UPPER}:")
    #MESSAGE(STATUS "  QT_${_COMPONENT_UPPER}_LIBRARY: ${QT_${_COMPONENT_UPPER}_LIBRARY}")
    #MESSAGE(STATUS "  QT_${_COMPONENT_UPPER}_INCLUDE_DIR: ${QT_${_COMPONENT_UPPER}_INCLUDE_DIR}")
    #MESSAGE(STATUS "  QT_${_COMPONENT_UPPER}_FOUND: ${QT_${_COMPONENT_UPPER}_FOUND}")

    SET(QT_INCLUDES ${QT_INCLUDES} ${QT_${_COMPONENT_UPPER}_INCLUDE_DIR})
    SET(QT_LIBRARIES ${QT_LIBRARIES} ${QT_${_COMPONENT_UPPER}_INCLUDE_DIR})
    MARK_AS_ADVANCED(QT_${_COMPONENT_UPPER}_LIBRARY QT_${_COMPONENT_UPPER}_INCLUDE_DIR)
  ENDIF(NOT QT_${_COMPONENT_UPPER}_FOUND)
ENDFOREACH(_COMPONENT)

#MESSAGE(STATUS "QT_LIBRARIES: ${QT_LIBRARIES}")
#MESSAGE(STATUS "QT_INCLUDES: ${QT_INCLUDES}")

STRING(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" QT_VERSION_MAJOR "${QTVERSION}")
STRING(REGEX REPLACE "^[0-9]+\\.([0-9])+\\.[0-9]+.*" "\\1" QT_VERSION_MINOR "${QTVERSION}")
STRING(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" QT_VERSION_PATCH "${QTVERSION}")

IF(NOT QT_INCLUDE_DIR)
  EXEC_PROGRAM(${QT_QMAKE_EXECUTABLE} ARGS "-query QT_INSTALL_HEADERS" OUTPUT_VARIABLE QTHEADERS)
  SET(QT_INCLUDE_DIR ${QTHEADERS} CACHE INTERNAL "" FORCE)
ENDIF(NOT QT_INCLUDE_DIR)

IF(NOT QT_LIBRARY_DIR)
  EXEC_PROGRAM(${QT_QMAKE_EXECUTABLE} ARGS "-query QT_INSTALL_LIBS" OUTPUT_VARIABLE QTLIBS)
  SET(QT_LIBRARY_DIR ${QTLIBS} CACHE INTERNAL "" FORCE)
ENDIF(NOT QT_LIBRARY_DIR)

IF(NOT QT_BINARY_DIR)
  EXEC_PROGRAM(${QT_QMAKE_EXECUTABLE} ARGS "-query QT_INSTALL_BINS" OUTPUT_VARIABLE QTBINS)
  SET(QT_BINARY_DIR ${QTBINS} CACHE INTERNAL "" FORCE)
ENDIF(NOT QT_BINARY_DIR)

IF(NOT QT_DOC_DIR)
  EXEC_PROGRAM(${QT_QMAKE_EXECUTABLE} ARGS "-query QT_INSTALL_DOCS" OUTPUT_VARIABLE QTDOCS)
  SET(QT_DOC_DIR ${QTDOCS} CACHE INTERNAL "" FORCE)
ENDIF(NOT QT_DOC_DIR)

IF(NOT QT_MOC_EXECUTABLE)
  FIND_PROGRAM(QT_MOC_EXECUTABLE NAMES moc moc5 moc-qt5 PATHS ${QT_BINARY_DIR}
               NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
ENDIF(NOT QT_MOC_EXECUTABLE)

MARK_AS_ADVANCED(QT_INCLUDES QT_INCLUDE_DIR
                 QT_LIBRARIES QT_LIBRARY_DIR
                 QT_BINARY_DIR
                 QT_DOC_DIR
                 QT_QMAKE_EXECUTABLE_FINDQT QT_QMAKE_EXECUTABLE QT_MOC_EXECUTABLE)

# Invokes pkgconfig, cleans up the result and sets variables
#EXECUTE_PROCESS(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable qt_config QtCore
#    OUTPUT_VARIABLE _pkgconfig_flags
#    RESULT_VARIABLE _pkgconfig_failed)
#STRING(REPLACE " " ";" QT_CONFIG_FLAGS "${_pkgconfig_flags}")

INCLUDE(Qt5Macros)

SET(QT_FOUND TRUE)
SET(QT5_FOUND TRUE)

