#!/bin/bash
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

if [ -z $USING_S3SYNC ] ; then
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
  
  # rsync elevation data:
  rsync -rm --delete $OSSIM_DATA_REPOSITORY/elevation/dted/level0 $OSSIM_DATA/elevation/dted
  if [ $? != 0 ]; then 
    echo "ERROR: Failed data repository rsync of elevation."
    exit 1
  fi
  
  # rsync nadcon data:
  rsync -rm --delete $OSSIM_DATA_REPOSITORY/elevation/nadcon $OSSIM_DATA/elevation
  if [ $? != 0 ]; then 
    echo "ERROR: Failed data repository rsync of nadcon grids."
    exit 1
  fi
  
  # rsync geoid 96 data:
  rsync -rm --delete $OSSIM_DATA_REPOSITORY/elevation/geoid96_little_endian/ $OSSIM_DATA/elevation/geoids/geoid96
  if [ $? != 0 ]; then 
    echo "ERROR: Failed data repository rsync of geoid96 grids."
    exit 1
  fi
  
  # rsync geoid 99 data:
  rsync -rm --delete $OSSIM_DATA_REPOSITORY/elevation/geoid99_little_endian/ $OSSIM_DATA/elevation/geoids/geoid99
  if [ $? != 0 ]; then 
    echo "ERROR: Failed data repository rsync of geoid99 grids."
    exit 1
  fi
  
  #rsync imagery
  rsync -rm --delete $OSSIM_DATA_REPOSITORY/test/data/public/ $OSSIM_DATA/ossim_data
  if [ $? != 0 ]; then 
    echo "ERROR: Failed data repository rsync of imagery."
    exit 1
  fi
fi
   
exit 0
