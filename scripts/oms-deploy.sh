#!/bin/bash 
pushd $(dirname ${BASH_SOURCE[0]}) > /dev/null
OSSIMCI_SCRIPT_DIR=`pwd -P`
popd >/dev/null

source $OSSIMCI_SCRIPT_DIR/ossim-env.sh

pushd $OSSIM_DEV_HOME/ossim-oms/joms
   gradle uploadArchives
   if [ $? -ne 0 ]; then
     echo; echo "ERROR: Build failed for JOMS Deploy to Nexus."
     exit 1
   fi
 popd
