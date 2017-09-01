#!/bin/bash

#----------------------------------------------------------------------------------
# Git remove branch. Deletes local AND upstream origin/branch.
#----------------------------------------------------------------------------------

if [ -z $1 ]; then
  echo; echo "git remove branch: need branch name."; echo
  exit 0
fi 

branchname=$1
echo 
read -p "Are you sure you want to completely delete $branchname? [y/N]" yesno
if [ -z $yesno ]; then
  yesno="no"
fi

if [ $yesno == "y" ] || [ $yesno == "Y" ]; then
  git branch -d $branchname
  if [ $? -ne 0 ]; then
    echo; echo "Local branch not deleted."; echo
    exit 1
  fi
  git push origin --delete $branchname
  if [ $? -ne 0 ]; then
    echo; echo "Remote branch not deleted."; echo
    exit 1
  fi
fi

echo; echo "Branch $branchname successfully removed."; echo
exit 0

      

