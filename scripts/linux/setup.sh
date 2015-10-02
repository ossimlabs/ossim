#!/bin/sh
###############################################################################
#
# Setup script for all OSSIM repositories
#
# The test data directory, specified by the env var OSSIM_BATCH_TEST_DATA is 
# syncronized against a master repository.
# 
# OSSIM_BATCH_TEST_DATAmust be defined prior.
#
###############################################################################
# Sync against S3 for test data:
if [ -z $OSSIM_BATCH_TEST_DATA ] || [ ! -d $OSSIM_BATCH_TEST_DATA ] ; then
  echo "ERROR: Env var OSSIM_BATCH_TEST_DATA=$OSSIM_BATCH_TEST_DATA does not point to a valid data directory. Aborting setup..."; 
  exit 1
fi

echo; echo "STATUS: Syncing data directory to S3..."
s3cmd sync --no-check-md5 s3://yumrepos-dev-rbtcloud/ossim_data/public $OSSIM_BATCH_TEST_DATA/
if [ $? != 0 ]; then
  echo "ERROR: Failed S3 sync."
  exit 1
fi
echo "STATUS: S3 sync successful..."

exit 0

