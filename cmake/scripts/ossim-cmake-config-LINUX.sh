#!/bin/bash

##########################################################################
#
# CMake Configuration for OSSIM on Linux
#
# DO NOT RELOCATE THIS SCRIPT. Its location is used to resolve relative
# directory paths.
#
# Usage: <this_script_name> [<build_type>] 
#
# where the optional <build_type> is one of the following literals:
# "Release" (default), "Debug", "RelWithDebInfo", or "MinSizeRel"
#
# If a build type = "eclipse" is specified, cmake will generate a Debug
# build environment along with Eclipse CDT4 project files (in the build
# directory) for importing as existing project.
#
# Instructions: Run this script to build the repository. This script can be
# run from any directory, but will assume a default relative directory
# heirarchy when searching for related items.
# 
# No env vars need to be predefined. The build output will be written to
# $OSSIM_DEV_HOME/build/<build_type> where $OSSIM_DEV_HOME is the top-level
# folder containing all OSSIM repositories (including this one).
#
# For customized output location, you can define the env var OSSIM_BUILD_DIR
# prior to running this script, and the output will be written there.
#
##########################################################################

# Uncomment following line to debug script line by line:
#set -x; trap read debug

# Fetch the build type from command line:
BUILD_TYPE_ARG=${1^^}
case "$BUILD_TYPE_ARG" in
  DEBUG)
      CMAKE_BUILD_TYPE="Debug"
      ;;
  ECLIPSE)
      CMAKE_BUILD_TYPE="Debug"
      ;;
  RELEASE)
      CMAKE_BUILD_TYPE="Release"
      ;;
  RELWITHDEBINFO)
      CMAKE_BUILD_TYPE="RelWithDebInfo"
      ;;
  MINSIZEREL)
      CMAKE_BUILD_TYPE="MinSizeRel"
      ;;
  *)
      CMAKE_BUILD_TYPE="Release"
esac

# Establish location of master CMakeLists.txt file. This is the cmake file
# used to build all OSSIM-related repos (plugins, tests, oms, etc)
SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
pushd $SCRIPT_DIR/..
  CMAKE_DIR=$PWD
popd

# Establish the top-level directory above repo containing this script
pushd $CMAKE_DIR/../..
OSSIM_DEV_HOME=$PWD
popd

# Establish CMake's output build directory:
if [ -z $OSSIM_BUILD_DIR ]; then
  OSSIM_BUILD_DIR=$OSSIM_DEV_HOME/build
fi

# Additional stuff for ECLIPSE CDT4 users:
CMAKE_G_ARG="Unix Makefiles"
if [ $BUILD_TYPE_ARG == "ECLIPSE" ]; then
  CMAKE_G_ARG="Eclipse CDT4 - Unix Makefiles"
  cp -f $CMAKE_DIR/CMakeLists.txt $OSSIM_DEV_HOME
  CMAKE_DIR=$OSSIM_DEV_HOME
fi

mkdir -p $OSSIM_BUILD_DIR
pushd $OSSIM_BUILD_DIR
rm CMakeCache.txt

echo "Generating Makefiles in" $OSSIM_BUILD_DIR

# CMAKE command 
cmake -G "$CMAKE_G_ARG" \
-DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
-DOSSIM_DEV_HOME=$OSSIM_DEV_HOME \
$CMAKE_DIR

popd

