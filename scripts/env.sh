#!/bin/bash 
pushd `dirname $0` >/dev/null
export SCRIPT_DIR=`pwd -P`
popd >/dev/null

if [ -z "$OSSIM_DEV_HOME" ]; then
   pushd $SCRIPT_DIR/../.. >/dev/null
   export OSSIM_DEV_HOME=$PWD
   popd >/dev/null
fi
if [ -z "$OSSIM_MAKE_JOBS" ]; then
   export OSSIM_MAKE_JOBS=4
fi

if [ -z "$OSSIM_INSTALL_PREFIX" ]; then
   export OSSIM_INSTALL_PREFIX=$OSSIM_DEV_HOME/install
fi
if [ -z "$OSSIM_BUILD_DIR" ]; then
   export OSSIM_BUILD_DIR=$OSSIM_DEV_HOME/build
fi
export CMAKE_CONFIG_SCRIPT=$OSSIM_DEV_HOME/ossim/cmake/scripts/ossim-cmake-config.sh
