#!/bin/bash

######################################################################################
#
# Test script for all OSSIM repositories. Required environment variables are:
#
#   OSSIM_DATA -- Local directory to contain elevation, imagery, and expected results
#   OSSIM_INSTALL_PREFIX -- Top level directory containing OSSIM installation.
#   OSSIM_PREFS_FILE -- Path to preferences used by test executables.
#
# Optional environment variables are:
#
#   OSSIM_BATCH_TEST_DATA -- Defaults to $OSSIM_DATA/ossim_data
#   OSSIM_BATCH_TEST_RESULTS -- Defaults to $OSSIM_DATA/test_results
#   GENERATE_EXPECTED_RESULTS -- "true"|"false". Defaults to "false"
# 
# Usage: test.sh [genx]
# 
# The required environment variable OSSIM_DATA should also ontain elevation data as 
# specified in the ossim_preferences file.
#
# The test data referenced must be available at $OSSIM_BATCH_TEST_DATA. This 
# environment variable can be predefined, otherwise, will default to:
# $OSSIM_DATA/ossim_data.
#
# The expected results should be in $OSSIM_BATCH_TEST_EXPECTED. This environment 
# variable can be predefined, otherwise, will default to
# $OSSIM_BATCH_TEST_DATA/expected_results.
#
# If the optional "genx" argument is specified, or $OSSIM_BATCH_TEST_EXPECTED does not
# exist, then expected results will be generated.
#
######################################################################################
#set -x; trap read debug

echo; echo "Running test.sh out of <$PWD>"

echo "Checking for required environment variables..."
if [ ! -d $OSSIM_DATA ]; then
  echo "ERROR: Required env var OSSIM_DATA is not defined or directory does not exist. Aborting setup..."; 
  exit 1
fi
if [ ! -d $OSSIM_INSTALL_PREFIX ]; then
  echo "ERROR: Required env var OSSIM_INSTALL_PREFIX is not defined or directory does not exist. Aborting setup..."; 
  exit 1
fi
if [ ! -f $OSSIM_PREFS_FILE ]; then
  echo "ERROR: Required env var OSSIM_PREFS_FILE is not defined or file does not exist. Aborting setup..."; 
  exit 1
fi

if [ -z $OSSIM_BATCH_TEST_DATA ]; then
  export OSSIM_BATCH_TEST_DATA=$OSSIM_DATA/ossim_data
fi

if [ -z $OSSIM_BATCH_TEST_RESULTS ]; then
  export OSSIM_BATCH_TEST_RESULTS=$OSSIM_DATA/test_results
fi

echo; echo "Test Environment:"
echo "  OSSIM_DATA=$OSSIM_DATA"
echo "  OSSIM_INSTALL_PREFIX=$OSSIM_INSTALL_PREFIX"
echo "  OSSIM_PREFS_FILE=$OSSIM_PREFS_FILE"
echo "  OSSIM_BATCH_TEST_DATA=$OSSIM_BATCH_TEST_DATA"
echo "  OSSIM_BATCH_TEST_RESULTS=$OSSIM_BATCH_TEST_RESULTS"
echo

if [ ! -d $OSSIM_BATCH_TEST_RESULTS ]; then
  echo "STATUS: No test results were detected. Creating directory.";
  mkdir -p $OSSIM_BATCH_TEST_RESULTS;
fi
if [ ! -d $OSSIM_BATCH_TEST_RESULTS/exp ]; then
  echo "STATUS: No expected results were detected. Will generating expected results.";
  export GENERATE_EXPECTED_RESULTS="true";
  mkdir -p $OSSIM_BATCH_TEST_RESULTS/exp;
fi

#export the OSSIM runtime env to child processes:
export PATH=$OSSIM_INSTALL_PREFIX/bin:$PATH
export LD_LIBRARY_PATH=$OSSIM_INSTALL_PREFIX/lib:$LD_LIBRARY_PATH

# TEST 1: Check ossim-info version:
echo; echo "STATUS: Running ossim-info test...";
COMMAND1="ossim-info --config --plugins"
$COMMAND1
if [ $? -ne 0 ]; then
  echo; echo "ERROR: Failed while attempting to run <$COMMAND1>."
  exit 1
fi

COUNT=`ossim-info --version | grep --count "ossim-info 1.9"`
if [ $COUNT != "1" ]; then
  echo "FAIL: Failed ossim-info test"; exit 1
else
  echo "STATUS: Passed ossim-info test"
fi

if [ $GENERATE_EXPECTED_RESULTS -eq "true" ]; then
  pushd $SCRIPT_DIR/../../test/scripts
  echo "STATUS: Running ossim-batch-test --accept-test super-test.kwl...";echo
  ossim-batch-test --accept-test all super-test.kwl
  EXIT_CODE=$?
  popd
  #echo "STATUS: ossim-batch-test exit code = $?";echo
  if [ $EXIT_CODE != 0 ]; then
    echo "FAIL: Error encountered generating expected results."
    exit 1
  else
    echo "STATUS: Successfully generated expected results."; echo
  fi

else
  # Run batch tests
  echo; echo "STATUS: Running batch tests..."
  pushd $SCRIPT_DIR/../../test/scripts
  ossim-batch-test super-test.kwl
  EXIT_CODE=$?
  popd
  #echo "#### DEBUG #### EXIT_CODE = $EXIT_CODE"
  if [ $EXIT_CODE != 0 ]; then
    echo "FAIL: Failed batch test"
    exit 1
  else
    echo "STATUS: Passed batch test"
  fi

fi


# Success!
echo "STATUS: Passed all tests."
exit 0

