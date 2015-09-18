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

# TEST 1: Check ossim-info version:
COUNT=`/var/lib/go-agent/pipelines/ossimlabs-pipeline/build/bin/ossim-info --version | grep --count "ossim-info 1.9"`
if [ $COUNT != "1" ]; then
  echo "Failed TEST 1"; exit 1
fi

# Success!
echo "Passed all tests."
exit 0

