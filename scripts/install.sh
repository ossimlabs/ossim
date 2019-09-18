#!/bin/bash 
pushd `dirname ${BASH_SOURCE[0]}` >/dev/null
export INTALL_SCRIPT_DIR=`pwd -P`
popd >/dev/null
# source variables used during the builds
. $INTALL_SCRIPT_DIR/env.sh

pushd $OSSIM_BUILD_DIR >/dev/null
make install
popd >/dev/null

pushd $OSSIM_DEV_HOME/ossim
  
popd
