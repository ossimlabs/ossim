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
# Instructions: Run this script to build the repository. This script can be
# run from any directory, but will assume a default relative directory
# heirarchy when searching for related items.
# 
##########################################################################

# Uncomment following line to debug script line by line:
# set -x; trap read debug

# Fetch the build type from command line:
CMAKE_BUILD_TYPE=$1
if [ -z $CMAKE_BUILD_TYPE ]; then
  CMAKE_BUILD_TYPE="Release"
fi

# Establish the top-level directory of repo containing this script
pushd `dirname $0`/../..
REPO_TOP_DIR=$PWD
popd

# Establish CMake's output build directory based on build type:
if [ -z $OSSIM_BUILD_DIR ]; then
  OSSIM_BUILD_DIR=$REPO_TOP_DIR/build/$CMAKE_BUILD_TYPE
fi

mkdir -p $OSSIM_BUILD_DIR
pushd $OSSIM_BUILD_DIR
rm CMakeCache.txt
echo "Generating Makefiles in" $OSSIM_BUILD_DIR

# CMAKE command 
cmake -G "Unix Makefiles" \
-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
-DBUILD_OSSIM_APPS=ON \
-DBUILD_OSSIM_TESTS=ON \
${REPO_TOP_DIR}/build

popd

