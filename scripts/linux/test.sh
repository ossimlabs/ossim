#!/bin/sh

###############################################################################
#
# Usage: test.sh [genx]
# 
# Test script for all OSSIM repositories. The test data referenced must be 
# available at $OSSIM_BATCH_TEST_DATA, which must be defined prior.
#
# The expected results should be in $OSSIM_BATCH_TEST_RESULTS. This environment 
# variable can be predefined, otherise, will default to:
#
#    $OSSIM_BATCH_TEST_DATA/ossim-test-results
#
# If the optional "genx" argument is specified, then expected results will be
# generated at $OSSIM_BATCH_TEST_RESULTS ONLY IF this directory is not present.
# If $OSSIM_BATCH_TEST_RESULTS exists, no expected results will be generated.
#
###############################################################################

if [ $1 == "genx" ]; then
  GENERATE_EXPECTED_RESULTS=1
fi

if [ -z $OSSIM_BUILD_DIR ]; then
  export OSSIM_BUILD_DIR=$PWD/build
fi

if [ -z $OSSIM_BATCH_TEST_DATA ]; then
  echo "ERROR: Required env var OSSIM_BATCH_TEST_DATA is not defined. Aborting setup..."; 
  exit 1
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
    mkdir $OSSIM_BATCH_TEST_RESULTS
    if [ $? -ne 0 ]; then
      echo; echo "Failed while attempting to create results directory at <$OSSIM_BATCH_TEST_RESULTS>. Check permissions."
      echo 1
    fi
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

