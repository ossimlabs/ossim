#!/bin/sh
###############################################################################
#
# Setup script for all OSSIM repositories
#
# The test data directory, specified by the env var OSSIM_DATA is
# syncronized against a master repository.
#
# If OSSIM_DATA is not set or the defined directory doesn't exist
# This will create the dir and sync the data from S3.  NOTE: s3cmd credentials
# must be stored on the machine that this test needs to run.
#
###############################################################################

if [ -z $OSSIM_DATA ] || [ ! -d $OSSIM_DATA ] ; then
  mkdir -p /data/ossim_data
  export OSSIM_DATA=/data/ossim_data
if

if [ -z $USING_S3 ] ; then
  echo; echo "STATUS: Syncing data directory to S3..."
  s3cmd sync --no-check-md5 s3://yumrepos-dev-rbtcloud/ossim_data/public $OSSIM_DATA/
  if [ $? != 0 ]; then
    echo "ERROR: Failed S3 sync."
    exit 1
  fi
else
  echo; echo "STATUS: Syncing data directory to data repository..."
  if [ ! -z $OSSIM_DATA_REPOSITORY ] ; then
    echo "ERROR: Env var OSSIM_DATA_REPOSITORY must be defined in order to syncronize against data repository."
    exit 1
  fi
  if [ ! -d $OSSIM_DATA_REPOSITORY ] ; then
    echo "ERROR: $OSSIM_DATA_REPOSITORY is not a valid directory/mount point."
    exit 1
  fi
  rsync -r --delete $OSSIM_DATA_REPOSITORY $OSSIM_DATA
  if [ $? != 0 ]; then 
    echo "ERROR: Failed data repository rsync."
    exit 1
  fi
fi
   
exit 0
