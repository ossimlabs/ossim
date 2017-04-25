#!/bin/bash

#----------------------------------------------------------------------------------
# Git create branch. Also creates upstream origin/branch and switches to new branch
#----------------------------------------------------------------------------------

if [ -z $1 ]; then
  echo; echo "git remove branch: need branch name."; echo
  exit 0
fi 

branchname=$1
git checkout -b $branchname
if [ $? -ne 0 ]; then
  echo; echo "Local branch not created."; echo
  exit 1
fi
git push -u origin  $branchname
if [ $? -ne 0 ]; then
  echo; echo "Remote branch not created."; echo
  exit 1
fi
echo; echo "Branch $branchname successfully created."; echo

exit 0

      

