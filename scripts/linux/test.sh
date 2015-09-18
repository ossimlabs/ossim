#!/bin/sh

# TEST 1: Check ossim-info version:
COUNT=`build/bin/ossim-info --version | grep --count "ossim-info 1.9"`
if [ $COUNT != "1" ]; then
  echo "Failed TEST 1"; exit 1
fi

# Success!
echo "Passed all tests."
exit 0

