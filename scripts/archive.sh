#!/bin/bash

#---
# File: archive.sh
# 
# Description: Convenience script to create a tar ball suitable for rpmbuild.
# This script will create a tarball, e.g. ossim-1.9.0.tar.gz 
# from a the top level set of ossim git modules, i.e. ossim_labs_dev_root.
#---

if [ $# -ne 3 ]
then
  echo "Usage:    $(basename $0) <path_to_ossimlabs_dev_root> <version> <branch>"
  echo "Example:  $(basename $0) ossimlabs 1.9.0 dev"
  echo "Where:    ossimlabs is directory containing individual git modules."
  echo -n "Creates:  ossimlabs/ossim-1.9.0.tar.gz suitable for building rpms with "
  echo "rpmbuild."
  exit 1
fi

dev_root=$1
version=$2
branch=$3
archive="ossim-$version"


echo "dev_root:    $dev_root"
echo "version:     $version"
echo "archive:     $archive"
echo "branch:      $branch"
echo ""

while true; do
   read -p "Continue? [y/n] " yn
   case $yn in
      [Yy]* ) break;;
      [Nn]* ) exit;;
          * ) echo "Please answer yes or no.";;
   esac
done


function archiveModule()
{
   dir=$(pwd)
   module=$1
   echo "dev_root: $dev_root"
   if [ -d $dev_root/$module ]; then
      cd $dev_root
      
      command="mkdir -p $archive/$module"
      echo $command
      $command
      
      cd $module
      git archive $branch | tar -x -C ../$archive/$module
      cd $dir
   else
       echo "Input module does not exist: $dev_root/$module"
   fi
}

archiveModule ossim
archiveModule ossim-gui
archiveModule ossim-oms
archiveModule ossim-planet
archiveModule ossim-plugins
archiveModule ossim-video
archiveModule ossim-wms
archiveModule ossim-GoCD
   
dir=$(pwd)
if [ -d $dev_root/$archive ]; then

   echo "created archive dir: $dev_root/$archive"

   cd $dev_root
   tar cvzf $archive.tar.gz $archive
   if [ -f $archive.tar.gz ]; then
      echo "wrote file: $dev_root/$archive.tar.gz"
   else
      echo "Error creating tar file!"
   fi
  
   cd $dir
fi

exit
