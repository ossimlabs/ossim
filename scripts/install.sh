#!/bin/bash 
pushd `dirname $0` >/dev/null
export SCRIPT_DIR=`pwd -P`
popd >/dev/null
# source variables used during the builds
. $SCRIPT_DIR/env.sh

pushd $OSSIM_BUILD_DIR >/dev/null
make install
popd >/dev/null

pushd $OSSIM_DEV_HOME/ossim
  
popd
