#!/bin/sh
###############################################################################
#
# Setup script for all OSSIM repositories
#
# The test data directory, specified by the env var OSSIM_BATCH_TEST_DATA is
# syncronized against a master repository.
#
# OSSIM_BATCH_TEST_DATA must be defined prior.
# If OSSIM_DATA is not set or the defined directory doesn't exist
# This will create the dir and sync the data from S3.  NOTE: s3cmd credentials
# must be stored on the machine that this test needs to run.
#
###############################################################################
# Sync against S3 for test data:

if [ -z $OSSIM_DATA ] || [ ! -d $OSSIM_DATA ] ; then
  mkdir -p /data/ossim_data
  export OSSIM_DATA=/data/ossim_data
  echo; echo "STATUS: Syncing data directory to S3..."
  s3cmd sync --no-check-md5 s3://yumrepos-dev-rbtcloud/ossim_data/public $OSSIM_DATA/
  if [ $? != 0 ]; then
    echo "ERROR: Failed S3 sync."
    exit 1
  fi
fi

exit 0
