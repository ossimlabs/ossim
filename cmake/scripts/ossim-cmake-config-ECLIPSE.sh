#!/bin/bash

##########################################################################
#
# CMake Configuration for OSSIM on Linux also generates Eclipse CDT4 
# project files. You can import the project into Eclipse as "Existing 
# Project into Workspace," and navigate to the output build directory
# containing the Eclipse project files.
#
# DO NOT RELOCATE THIS SCRIPT. Its location is used to resolve relative
# directory paths.
#
# Usage: <this_script_name>
#
# Instructions: Run this script to build the Makefiles. This script can be
# run from any directory, but will assume a default relative directory
# heirarchy when searching for related items.
# 
# No env vars need to be predefined. The build output will be written to
# $OSSIMLABS_DIR/build/Debug where $OSSIMLABS_DIR is the top-level
# folder containing all OSSIM repositories (including this one).
#
# For customized output location, you can define the env var OSSIM_BUILD_DIR
# prior to running this script, and the output will be written there.
#
##########################################################################

# Uncomment following line to debug script line by line:
#set -x; trap read debug

# Fetch the build type from command line:
CMAKE_BUILD_TYPE="Debug"

# Establish location of master CMakeLists.txt file. This is the cmake file
# used to build all OSSIM-related repos (plugins, tests, oms, etc)
pushd `dirname $0`/..
CMAKE_DIR=$PWD
popd

# Establish the top-level directory above repo containing this script
pushd $CMAKE_DIR/../..
export OSSIM_DEV_HOME=$PWD
popd

# Establish CMake's output build directory based on build type:
if [ -z $OSSIM_BUILD_DIR ]; then
  OSSIM_BUILD_DIR=$OSSIM_DEV_HOME/../build/$CMAKE_BUILD_TYPE
fi

mkdir -p $OSSIM_BUILD_DIR
pushd $OSSIM_BUILD_DIR
rm CMakeCache.txt

# Copy the master cmake file to the build dir:
cp -f $CMAKE_DIR/CMakeLists.txt $OSSIM_DEV_HOME

echo "Generating Makefiles in" $OSSIM_BUILD_DIR

cmake -G "Eclipse CDT4 - Unix Makefiles" \
-DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
-DBUILD_OSSIM_APPS=ON \
-DBUILD_OSSIM_TESTS=ON \
$OSSIM_DEV_HOME

popd

