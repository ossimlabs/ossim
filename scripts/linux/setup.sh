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
exit 0

