#!/bin/bash

# OCPLD -- Ossim CoPy Library Dependencies
# Adapted from code written by Hemanth.HM

# Uncomment to step/debug
#set -x; trap read debug
pushd `dirname ${BASH_SOURCE[0]}` >/dev/null
export SCRIPT_DIR=`pwd -P`
popd >/dev/null

if [ $# -ne 2 ]
then
  echo "Usage: `basename $0` <ossim_build_dir> <sandbox_dir>"
  exit 1
fi

OSSIM_BUILD_DIR=$1
SANDBOX_DIR=$2

echo "Copying libraries..."
$SCRIPT_DIR/ocpld.sh $OSSIM_BUILD_DIR/lib $SANDBOX_DIR/lib
if [ $? -ne 0 ]; then
  echo; echo "Error encountered during ocpld."
  popd>/dev/null
  exit 1
fi

echo "Copying headers..."
ossim-header-crawl $OSSIM_BUILD_DIR $SANDBOX_DIR/include
if [ $? -ne 0 ]; then
  echo; echo "Error encountered during ossim-header-crawl."
  popd>/dev/null
  exit 1
fi

echo; echo "Sandbox of dependencies has been successfully created in $SANDBOX_DIR."; echo