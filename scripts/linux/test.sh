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
# Optional argument "genx" directs the script to generate expected results
# if none are detected at $OSSIM_BATCH_TEST_RESULTS (assigned here) in lieu of
# running tests.
#
###############################################################################

# Debug:
#echo "########## PWD=$PWD"
#echo "########## HOME=$HOME"
#echo "########## USER=`whoami`"

if [ $1 == "genx" ]; then
  GENERATE_EXPECTED_RESULTS=1
fi

if [ -z $OSSIM_BUILD_DIR ]; then
  export OSSIM_BUILD_DIR=$PWD/build
fi
if [ -z $OSSIM_BATCH_TEST_DATA ]; then
  export OSSIM_BATCH_TEST_DATA=$HOME/test_data
fi
if [ -z $OSSIM_BATCH_TEST_RESULTS ]; then
  export OSSIM_BATCH_TEST_RESULTS=$OSSIM_BATCH_TEST_DATA/ossim-test-results
fi

#echo "########## OSSIM_BUILD_DIR=$OSSIM_BUILD_DIR"
#echo "########## OSSIM_BATCH_TEST_DATA=$OSSIM_BATCH_TEST_DATA"
#echo "########## OSSIM_BATCH_TEST_RESULTS=$OSSIM_BATCH_TEST_RESULTS"

#export the OSSIM runtime env to child processes:
export PATH=$OSSIM_BUILD_DIR/bin:$PATH
export LD_LIBRARY_PATH=$OSSIM_BUILD_DIR/lib:$LD_LIBRARY_PATH

# TEST 1: Check ossim-info version:
echo; echo "STATUS: Running ossim-info test...";echo
COUNT=`ossim-info --version | grep --count "ossim-info 1.9"`
if [ $COUNT != "1" ]; then
  echo "Failed TEST 1"; exit 1
fi


if [ $GENERATE_EXPECTED_RESULTS -eq 1 ]; then

  # Check if expected results are present, generate if not:
  if [ ! -e $OSSIM_BATCH_TEST_RESULTS ]; then
    echo; echo "STATUS: No expected results detected, generating new expected results in $OSSIM_BATCH_TEST_RESULTS..."
    pushd ossim/test/scripts
    ossim-batch-test --accept-test all super-test.kwl
    popd
    echo "STATUS: ossim-batch-test exit code = $?";echo
    if [ $? != 0 ]; then
      echo "Failed batch test generating expected results."
      exit 1
    fi
  fi

else

  # Run batch tests
  echo; echo "STATUS: Running batch tests..."
  pushd ossim/test/scripts
  ossim-batch-test super-test.kwl
  echo "STATUS: ossim-batch-test exit code = $?";echo
  if [ $? != 0 ]; then
    echo "Failed batch test"
    exit 1
  fi
fi

# Success!
echo "Passed all tests."
exit 0

