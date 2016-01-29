#!/bin/bash

#---
# File: archive.sh
# 
# Description: Convenience script to create a tar ball suitable for rpmbuild.
# This script will create a tarball, e.g. ossim-1.9.0.tar.gz 
# from a the top level set of ossim git modules, i.e. ossim_labs_dev_root.
#---

if [ $# -ne 5 ]
then
  echo "Usage:    $(basename $0) <remote_url> <output_dir> <version> <branch> <path_to_kakadu_source>"
  echo "Example:  $(basename $0) https://github.com/ossimlabs ossimlabs 1.9.0 dev"
  echo "Where:    ossimlabs is directory to put archived files and tar ball."
  echo -n "Creates:  ossim-1.9.0.tar.gz suitable for building rpms with "
  echo "rpmbuild."
  exit 1
fi

remote=$1
output_dir=$2
version=$3
branch=$4
kakadu_src=$5

archive="ossim-${version}"
kakadu_archive="ossim-kakadu-plugin-${version}"

echo "remote:         $remote"
echo "output_dir      $output_dir"
echo "version:        $version"
echo "branch:         $branch"
echo "kakadu_src:     $kakadu_src"
echo "archive:        $archive"
echo "kakadu_archive: $kakadu_archive"

echo ""

while true; do
   read -p "Continue? [y/n] " yn
   case $yn in
      [Yy]* ) break;;
      [Nn]* ) exit;;
          * ) echo "Please answer yes or no.";;
   esac
done

if [ ! -d $output_dir ]; then
   command="mkdir -p $output_dir"
   echo $command
   $command

   if [ ! -d $output_dir ]; then
      echo "Could not create: $output_dir"
      exit
   fi
fi


function archiveModule()
{
   dir=$(pwd)
   module=$1

   cd $output_dir

   if [ -d $module ]; then
      command="rm -rf $module"
      echo $command
      $command
   fi

   command="svn export ${remote}/${module}/branches/${branch} $module"
   echo $command
   $command

   cd $dir
}

function createOssimTarball()
{
   if [ -d $output_dir ]; then

      dir=$(pwd)
      cd $output_dir

      # Make the tarball for main ossim rpm:
      tar cvzf $archive.tar.gz $archive
      if [ -f $archive.tar.gz ]; then
         echo "wrote file: $output_dir/$archive.tar.gz"
      else
         echo "Error creating ossim tar file!"
      fi

      cd $dir
   fi
}

function createKakaduTarball()
{
   if [ -d $output_dir ]; then
      dir=$(pwd)
      cd $output_dir

      # Make the tarball for kakadu rpm:
      if [ -d $kakadu_archive ]; then
         command="rm -rf $kakadu_archive"
         echo $command
         $command
      fi

      command="mkdir $kakadu_archive"
      echo $command
      $command

      if [ ! -d $kakadu_archive ]; then
         echo "Could not create: $dir"
         exit
      fi

      # cmake modules:
      command="cp -r ossim/cmake/CMakeModules $kakadu_archive/."
      echo $command
      $command

      # ossim-plugins/kakadu code:
      command="cp -r ossim-plugins/kakadu $kakadu_archive/"
      echo $command
      $command

      # Kakadu source:
      command="cp -r $kakadu_src $kakadu_archive/kakadu_src"
      echo $command
      $command
   
      tar cvzf ${kakadu_archive}.tar.gz $kakadu_archive
      if [ -f $kakadu_archive.tar.gz ]; then
         echo "wrote file: $output_dir/${kakadu_archive}.tar.gz"
      else
         echo "Error creating kakadu tar file!"
      fi
      
      cd $dir
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
createOssimTarball
createKakaduTarball

# End of script:
exit
