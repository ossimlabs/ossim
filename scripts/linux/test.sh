#!/bin/sh

###############################################################################
#
# Test script for all OSSIM repositories
#
# This script must be run from the top "ossimlabs" directory containing ossim, 
# and other OSSIM-related repositories. It performs three functions:
# 
# No env vars need to be predefined. 
#
###############################################################################

# Debug:
echo "########## PWD=$PWD"
echo "########## HOME=$HOME"

if [ -z $OSSIM_BUILD_DIR ]; then
  export OSSIM_BUILD_DIR=$PWD/build
fi
if [ -z $OSSIM_BATCH_TEST_DATA ]; then
  export OSSIM_BATCH_TEST_DATA=$HOME/test_data/ossim-test-data
fi
if [ -z $OSSIM_BATCH_TEST_RESULTS ]; then
  export OSSIM_BATCH_TEST_RESULTS=$OSSIM_BATCH_TEST_DATA
fi

echo "########## OSSIM_BUILD_DIR=$OSSIM_BUILD_DIR"
echo "########## OSSIM_BATCH_TEST_DATA=$OSSIM_BATCH_TEST_DATA"
echo "########## OSSIM_BATCH_TEST_RESULTS=$OSSIM_BATCH_TEST_RESULTS"


# TEST 1: Check ossim-info version:
COUNT=`$OSSIM_BUILD_DIR/bin/ossim-info --version | grep --count "ossim-info 1.9"`
if [ $COUNT != "1" ]; then
  echo "Failed TEST 1"; exit 1
fi


# Sync against S3 for test data:
s3cmd -c .s3cfg sync s3://yumrepos-dev-rbtcloud/ossim_data/ossim-test-data $HOME/test_data

# Run batch tests
pushd ossim/test/scripts
EXIT_CODE=`$OSSIM_BUILD_DIR/bin/ossim-batch-test super-test.kwl`
if [ $EXIT_CODE != 0 ]; then
  echo "Failed batch test"
  exit 1
fi

# Success!
echo "Passed all tests."
exit 0

