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
if [ -z $OSSIM_DATA ] || [ ! -d $OSSIM_DATA ] ; then
  echo "ERROR: Env var OSSIM_DATA=$OSSIM_DATA does not point to a valid data directory. Aborting setup...";
  exit 1
fi

exit 0
