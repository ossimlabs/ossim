#!/bin/sh
echo
echo "########## PWD=$PWD"
echo
echo "########## HOME=$HOME"
echo
echo "########## USER=`whoami`"
echo
echo "########## Listing of pipeline root <$PWD>:"
echo
echo `ls -la`
echo

# Sync against S3 for test data:
echo; echo "STATUS: Syncing data directory to S3..."
s3cmd sync --no-check-md5 s3://yumrepos-dev-rbtcloud/ossim_data/public $HOME/test_data
if [ $? != 0 ]; then
  echo "ERROR: Failed S3 sync."
  exit 1
fi
echo "STATUS: S3 sync successful..."

exit 0

