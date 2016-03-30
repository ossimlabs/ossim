#!/bin/bash
#===========================================================================================
#
# Script to download, build, and install (as a sandbox), all OSSIM dependencies in an 
# interactive fashion.
#
#===========================================================================================

# Working directory must be top-level dir:
SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
pushd $SCRIPT_DIR/../..
OSSIM_DEV_HOME=$PWD

if [ $# -ne 2 ]
then
  echo "Usage: `basename $0` <ossim_install_prefix> <config_dir>"
  exit 1
fi

OSSIM_INSTALL_PREFIX=$2
if [ -z "$OSSIM_INSTALL_PREFIX" ]; then
   echo "OSSIM_INSTALL_PREFIX environment variable is not set!  Exiting..."
   exit 1
fi

CONFIG_DIR=$3
if [ -z "$CONFIG_DIR" ]; then
   echo "CONFIG_DIR environment variable is not set!  Exiting..."
   exit 1
fi

OS=linux
THREADS=4

# Whether or not to clean package before building.
CLEAN=0

echo "OSSIM_DEV_HOME:       $OSSIM_DEV_HOME"
echo "OSSIM_INSTALL_PREFIX: $OSSIM_INSTALL_PREFIX"
echo "CONFIG_DIR:           $CONFIG_DIR"
echo "CLEAN flag:           $CLEAN"
echo ""

while true; do
   read -p "Continue? [y/n] " yn
   case $yn in
      [Yy]* ) break;;
      [Nn]* ) exit;;
          * ) echo "Please answer yes or no.";;
   esac
done

# Export for cmake scripts:
export OSSIM_DEV_HOME
export OSSIM_INSTALL_PREFIX
export CONFIG_DIR

#---
# Dependency links:
# Last updated: 10 June 2015
#---
CMAKE_BASE=cmake-3.2.3
CMAKE_URL=http://www.cmake.org/files/v3.2/$CMAKE_BASE.tar.bz2

FFMPEG_BASE=ffmpeg-2.4.10
FFMPEG_URL=http://ffmpeg.org/releases/$FFMPEG_BASE.tar.bz2

GDAL_BASE=gdal-1.11.2
GDAL_URL=http://download.osgeo.org/gdal/1.11.2/$GDAL_BASE.tar.gz

GEOS_BASE=geos-3.4.2
GEOS_URL=http://download.osgeo.org/geos/$GEOS_BASE.tar.bz2

GEOTIFF_BASE=libgeotiff-1.4.1
GEOTIFF_URL=http://download.osgeo.org/geotiff/libgeotiff/$GEOTIFF_BASE.tar.gz

GIT_BASE=git-1.8.5.2
GIT_URL=http://git-core.googlecode.com/files/$GIT_BASE.tar.gz

# HDF4_BASE=hdf-4.2.10
# HDF4_URL=http://www.hdfgroup.org/ftp/HDF/HDF_Current/src/$HDF4_BASE.tar.gz

HDF5_BASE=hdf5-1.8.15-patch1
HDF5_URL=http://www.hdfgroup.org/ftp/HDF5/current/src/$HDF5_BASE.tar.gz

LIBJPEG_TURBO_BASE=libjpeg-turbo-1.4.1
LIBJPEG_TURBO_URL=http://downloads.sourceforge.net/libjpeg-turbo/libjpeg-turbo-1.4.1.tar.gz

OSG_BASE=OpenSceneGraph-3.2.0
OSG_URL=http://www.openscenegraph.org/downloads/developer_releases/OpenSceneGraph-3.2.0.zip

OSSIM_BASE_URL=https://svn.osgeo.org/ossim/trunk
# OSSIM_BASE_URL=https://svn.osgeo.org/ossim/branches/v1.8.18

PODOFO_BASE=podofo-0.9.2
PODOFO_URL=http://downloads.sourceforge.net/podofo/$PODOFO_BASE.tar.gz

PROJ_BASE=proj-4.8.0
PROJ_URL=http://download.osgeo.org/proj/$PROJ_BASE.tar.gz

SZIP_BASE=szip-2.1
SZIP_URL=http://www.hdfgroup.org/ftp/lib-external/szip/2.1/src/$SZIP_BASE.tar.gz

TIFF_BASE=tiff-4.0.3
TIFF_URL=http://download.osgeo.org/libtiff/$TIFF_BASE.tar.gz

ZLIB_BASE=zlib-1.2.8
ZLIB_URL=http://zlib.net/$ZLIB_BASE.tar.gz

# Install useful system utilities:

YUM_CMD=$(which yum)
APT_GET_CMD=$(which apt-get)

answer=
echo -n "Install common system packages (must have sudo priveledges) [y/N]: "
read answer
if [ "$answer" == 'y' ]; then
  packages="aspell automake cmake cvs expat-devel freeglut-devel freetype-devel gcc-c++ gcc-gfortran git libcurl-devel libjpeg-turbo-devel libxml2-devel minizip-devel openssl openssl-devel qt-devel subversion xemacs xemacs-common xemacs-info xemacs-muse xemacs-packages-base xemacs-packages-extra yasm zlib-devel"
  if [[ ! -z $YUM_CMD ]]; then
    sudo yum install $packages
  elif [[ ! -z $APT_GET_CMD ]]; then
    sudo apt-get install $packages
  else
    echo "Cannot determine system install mechanism (expected yum or apt-get). You'll need to install them manually."; echo
  fi
  if [ $? -ne 0 ]; then
    echo "Error encountered during package installs. You'll need to install them manually."; echo
  fi
fi


#---
# Functions:
#----
# automake ./configure build:
function buildPackage()
{
   if [ -n $1 ]; then
      pkg=$1 # package
 
      if [ -d $OSSIM_DEV_HOME/$pkg/latest ]; then

         cd $OSSIM_DEV_HOME/$pkg/latest          
       
         # Check for previous build:
         if [ -f $OSSIM_DEV_HOME/$pkg/latest/Makefile ]; then
            if [ $CLEAN -eq 1 ]; then
               command="make clean"
               echo $command
               $command
            else
               echo "clean disabled..."
            fi
         fi

         CONFIG_FILE=$pkg-automake-config.sh

         if [ ! -f $OSSIM_DEV_HOME/$pkg/latest/$CONFIG_FILE ]; then
            command="cp $CONFIG_DIR/$CONFIG_FILE $OSSIM_DEV_HOME/$pkg/latest/."
            echo $command
            $command
         fi
       
         if [ -f $CONFIG_FILE ]; then
      
            ./$CONFIG_FILE

            # Some packages getting bad builds with threads...
            command="make -j $THREADS"
            # command="make"
            echo $command
            $command

            command="make install"
            echo $command
            $command

         else
            echo "Missing file: $OSSIM_DEV_HOME/$pkg/${pkg}-automake-config.sh"
            exit 1
         fi

         cd $OSSIM_DEV_HOME

      else
         echo "No directory: $OSSIM_DEV_HOME/$pkg/latest"
         exit 1
      fi

   fi
}

function buildCmakePackage()
{
   if [ -n $1 ]; then

      pkg=$1 # package
      build_dir=$OSSIM_DEV_HOME/build/build_$pkg
    
      if [ ! -d $build_dir ]; then
         command="mkdir -p $build_dir"
         echo $command
         $command
      fi
       
      if [ -d $build_dir ]; then

         cd $build_dir

         if [ -f $build_dir/Makefile ]; then
            if [ $CLEAN -eq 1 ]; then
               command="make clean"
               echo $command
               $command
            else
               echo "clean disabled..."
            fi
         fi

         if [ -f $build_dir/CMakeCache.txt ]; then
            command="rm $build_dir/CMakeCache.txt"
            echo $command
            $command
         fi
    
         if [ -f $CONFIG_DIR/$pkg-cmake-config.sh ]; then

            command="$CONFIG_DIR/$pkg-cmake-config.sh"
            echo $command
            $command

            command="make -j $THREADS"
            echo $command
            $command

            command="make install"
            echo $command
            $command

            cd $OSSIM_DEV_HOME
         else
            echo "Missing file: $CONFIG_DIR/$pkg-cmake-config.sh"
            exit 1
         fi
      else
         echo "No directory: $OSSIM_DEV_HOME/$pkg/build"
         exit 1
      fi
   else
      echo "buildCmakePackage ERROR no package arg!!!"
   fi
}

function checkoutPackages()
{
   #---
   # Some old OS's don't have git so do this first:
   #---
   GIT_CMD=$(which git)
   if [[ ! -z $GIT_CMD ]]; then
     echo -n "Check out and build git[y/n]: "
     read answer
     if [ "$answer" == 'y' ]; then
        echo "git..."
        cd $OSSIM_DEV_HOME
        mkdir -p $OSSIM_DEV_HOME/git
        cd $OSSIM_DEV_HOME/git
        wget $GIT_URL
        tar xvzf $GIT_BASE.tar.gz
        ln -s  $GIT_BASE latest
        buildPackage git
        cd $OSSIM_DEV_HOME
     fi
   fi

   #---
   # Set the path:
   #---
   echo -n "Add $OSSIM_INSTALL_PREFIX to path for this script[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      export PATH=$OSSIM_INSTALL_PREFIX:$PATH
      echo "New path: $PATH"
   fi

   #---
   # Some old OS's don't have cmake so do this second:
   #---
   echo -n "Check out cmake[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "cmake..."
      cd $OSSIM_DEV_HOME
      mkdir -p $OSSIM_DEV_HOME/cmake
      cd $OSSIM_DEV_HOME/cmake
      # git clone git://cmake.org/cmake.git cmake-git
      wget $CMAKE_URL
      bunzip2 $CMAKE_BASE.tar.bz2
      tar xvf $CMAKE_BASE.tar
      ln -s  $CMAKE_BASE latest
      cd $OSSIM_DEV_HOME
   fi

   echo -n "Check out ffmpeg[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then

      echo "ffmpeg..."
      cd $OSSIM_DEV_HOME
      mkdir -p $OSSIM_DEV_HOME/ffmpeg
      # git clone git://source.ffmpeg.org/ffmpeg.git ffmpeg-git
      cd $OSSIM_DEV_HOME/ffmpeg
      wget $FFMPEG_URL
      bunzip2 $FFMPEG_BASE.tar.bz2
      tar xvf $FFMPEG_BASE.tar
      ln -s  $FFMPEG_BASE latest
      cd $OSSIM_DEV_HOME
   fi

   echo -n "Check out gdal[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "gdal..."
      mkdir -p $OSSIM_DEV_HOME/gdal
      cd $OSSIM_DEV_HOME/gdal
      # svn co  http://svn.osgeo.org/gdal/trunk/gdal gdal-svn
      wget $GDAL_URL
      tar xvzf $GDAL_BASE.tar.gz
      ln -s $GDAL_BASE latest
      cd $OSSIM_DEV_HOME
   fi

   echo -n "Check out geos[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "geos..."
      mkdir -p $OSSIM_DEV_HOME/build/build_geos
      mkdir -p $OSSIM_DEV_HOME/geos
      cd $OSSIM_DEV_HOME/geos
      wget $GEOS_URL
      bunzip2 $GEOS_BASE.tar.bz2
      tar xvf $GEOS_BASE.tar
      ln -s $GEOS_BASE latest
      cd $OSSIM_DEV_HOME
   fi

   echo -n "Check out geotiff[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "geotiff..."
      mkdir -p $OSSIM_DEV_HOME/build/build_geotiff
      mkdir -p $OSSIM_DEV_HOME/geotiff
      cd $OSSIM_DEV_HOME/geotiff
      wget $GEOTIFF_URL
      tar xvzf $GEOTIFF_BASE.tar.gz
      ln -s $GEOTIFF_BASE latest
      cd $OSSIM_DEV_HOME
   fi

   echo -n "Check out hdf4[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "hdf4..."
      mkdir -p $OSSIM_DEV_HOME/build/build_hdf4
      mkdir -p $OSSIM_DEV_HOME/hdf4
      cd $OSSIM_DEV_HOME/hdf4
      wget $HDF4_URL
      tar xvzf $HDF4_BASE.tar.gz
      ln -s $HDF4_BASE latest
      cd $OSSIM_DEV_HOME
   fi

   echo -n "Check out hdf5[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "hdf5..."
      mkdir -p $OSSIM_DEV_HOME/build/build_hdf5
      mkdir -p $OSSIM_DEV_HOME/hdf5
      cd $OSSIM_DEV_HOME/hdf5
      wget $HDF5_URL
      tar xvzf $HDF5_BASE.tar.gz
      ln -s $HDF5_BASE latest
      cd $OSSIM_DEV_HOME
   fi

   echo -n "Check out libjpeg-turbo[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "geotiff..."
      mkdir -p $OSSIM_DEV_HOME/build/build_libjpeg-turbo
      mkdir -p $OSSIM_DEV_HOME/libjpeg-turbo
      cd $OSSIM_DEV_HOME/libjpeg-turbo
      wget $LIBJPEG_TURBO_URL
      tar xvzf $LIBJPEG_TURBO_BASE.tar.gz
      ln -s $LIBJPEG_TURBO_BASE latest
      cd $OSSIM_DEV_HOME
   fi

   answer=
   echo -n "Check out libwms[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "svn co ossimwms..."
      cd $OSSIM_DEV_HOME
      svn co $OSSIM_BASE_URL/libwms libwms
   fi

   answer=
   echo -n "Check out omar[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "git clone omar..."
      cd $OSSIM_DEV_HOME
      git clone https://github.com/radiantbluetechnologies/omar.git omar
   fi

   answer=
   echo -n "Check out oms[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "svn co oms..."
      cd $OSSIM_DEV_HOME
      svn co $OSSIM_BASE_URL/oms oms
   fi

   echo -n "Check out OpenSceneGraph[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "geos..."
      mkdir -p $OSSIM_DEV_HOME/build/build_osg
      mkdir -p $OSSIM_DEV_HOME/osg
      cd $OSSIM_DEV_HOME/osg
      wget $OSG_URL
      unzip $OSG_BASE.zip
      ln -s $OSG_BASE latest
      cd $OSSIM_DEV_HOME
   fi

   answer=
   echo -n "Check out ossim[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "svn co ossim..."
      cd $OSSIM_DEV_HOME
      svn co $OSSIM_BASE_URL/ossim ossim
   fi

   answer=
   echo -n "Check out ossimGui[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "svn co ossimGui..."
      cd $OSSIM_DEV_HOME
      svn co $OSSIM_BASE_URL/ossimGui ossimGui
   fi

   answer=
   echo -n "Check out ossimjni[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "svn co ossimjni..."
      cd $OSSIM_DEV_HOME
      svn co $OSSIM_BASE_URL/ossimjni ossimjni
   fi

   answer=
   echo -n "Check out ossim_junkyard[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "svn co ossimjunkyard..."
      cd $OSSIM_DEV_HOME
      svn co https://svn.osgeo.org/ossim/ossim_junkyard ossim_junkyard
   fi

   answer=
   echo -n "Check out ossimPlanet[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "svn co ossimPlanet..."
      cd $OSSIM_DEV_HOME
      svn co $OSSIM_BASE_URL/ossimPlanet ossimPlanet
   fi

   answer=
   echo -n "Check out ossimPlanetQt[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "svn co ossimPlanetQt..."
      cd $OSSIM_DEV_HOME
      svn co $OSSIM_BASE_URL/ossimPlanetQt ossimPlanetQt
   fi

   answer=
   echo -n "Check out ossimPredator[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "svn co ossimPredator..."
      cd $OSSIM_DEV_HOME
      svn co $OSSIM_BASE_URL/ossimPredator ossimPredator
   fi

   answer=
   echo -n "Check out ossim_package_support[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "svn co ossim_package_support..."
      cd $OSSIM_DEV_HOME
      svn co $OSSIM_BASE_URL/ossim_package_support ossim_package_support
   fi

   answer=
   echo -n "Check out ossim_plugins[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "svn co ossim_plugins..."
      cd $OSSIM_DEV_HOME
      svn co $OSSIM_BASE_URL/ossim_plugins ossim_plugins
   fi

   answer=
   echo -n "Check out ossim_qt4[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "svn co ossim_qt4..."
      cd $OSSIM_DEV_HOME
      svn co $OSSIM_BASE_URL/ossim_qt4 ossim_qt4
   fi

   echo -n "Check out pdal[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "pdal..."
      cd $OSSIM_DEV_HOME
      mkdir -p $OSSIM_DEV_HOME/pdal
      cd $OSSIM_DEV_HOME/pdal
      git clone https://github.com/PDAL/PDAL.git pdal-git
      ln -s pdal-git latest
      cd $OSSIM_DEV_HOME
   fi

   echo -n "Check out podofo[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "szip..."
      mkdir -p $OSSIM_DEV_HOME/build/build_podofo
      mkdir -p $OSSIM_DEV_HOME/podofo
      cd $OSSIM_DEV_HOME/podofo
      wget $PODOFO_URL
      tar xvzf $PODOFO_BASE.tar.gz
      ln -s $PODOFO_BASE latest
      cd $OSSIM_DEV_HOME
   fi

   answer=
   echo -n "Check out proj[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "svn co proj..."
      mkdir -p $OSSIM_DEV_HOME/proj
      cd $OSSIM_DEV_HOME/proj
      wget $PROJ_URL
      tar xvzf $PROJ_BASE.tar.gz
      ln -s $PROJ_BASE latest
      cd $OSSIM_DEV_HOME
   fi

   echo -n "Check out szip[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "szip..."
      mkdir -p $OSSIM_DEV_HOME/build/build_szip
      mkdir -p $OSSIM_DEV_HOME/szip
      cd $OSSIM_DEV_HOME/szip
      wget $SZIP_URL
      tar xvzf $SZIP_BASE.tar.gz
      ln -s $SZIP_BASE latest
      cd $OSSIM_DEV_HOME
   fi

   answer=
   echo -n "Check out tiff[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "tiff..."
      mkdir -p $OSSIM_DEV_HOME/tiff
      cd $OSSIM_DEV_HOME/tiff
      wget $TIFF_URL
      tar xvzf $TIFF_BASE.tar.gz
      ln -s $TIFF_BASE latest
      cd $OSSIM_DEV_HOME
   fi

   echo -n "Check out zlib[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      echo "zlib..."
      mkdir -p $OSSIM_DEV_HOME/build/build_zlib
      mkdir -p $OSSIM_DEV_HOME/zlib
      cd $OSSIM_DEV_HOME/zlib
      wget $ZLIB_URL
      tar xvzf $ZLIB_BASE.tar.gz
      ln -s $ZLIB_BASE latest
      cd $OSSIM_DEV_HOME
   fi

   #---
   # End of get code section:
   #---

} # End checkoutPackages(){...}

function buildPackages()
{
   #---
   # Build section:
   #
   # Note: Order dependent...
   #---
   echo -n "Build cmake[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      buildPackage cmake
   fi

   echo -n "Build libjpeg-turbo[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      buildPackage libjpeg-turbo
   fi

   echo -n "Build zlib[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      buildCmakePackage zlib
   fi

   echo -n "Build ffmpeg[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      buildPackage ffmpeg
   fi

   echo -n "Build geos[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      buildCmakePackage geos
   fi

   # Must build before hdf5 code:
   echo -n "Build szip[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      buildCmakePackage szip
   fi

   echo -n "Build hdf4[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      buildCmakePackage hdf4
   fi

   echo -n "Build hdf5[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      buildCmakePackage hdf5
   fi

   echo -n "Build tiff[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      buildPackage tiff
   fi

   # Must build before geotiff:
   echo -n "Build proj[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      buildPackage proj
      command="cp $OSSIM_DEV_HOME/proj/latest/src/projects.h $OSSIM_INSTALL_PREFIX/include/."
      echo $command
      $command
   fi

   echo -n "Build geotiff[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      buildCmakePackage geotiff
   fi

   echo -n "Build gdal[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      buildPackage gdal
   fi

   echo -n "Build OpenSceneGraph[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      buildCmakePackage osg
   fi

   echo -n "Buil2d podofo[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      buildCmakePackage podofo
   fi

   echo -n "Build pdal[y/n]: "
   read answer
   if [ "$answer" == 'y' ]; then
      buildCmakePackage pdal
   fi

   #---
   # End of build section:
   #---

} # End: buildPackages(){...}

answer=
echo -n "Go to check out packages section:[y/n]: "
read answer
if [ "$answer" == 'y' ]; then
   checkoutPackages
fi

answer=
echo -n "Go to build packages section:[y/n]: "
read answer
if [ "$answer" == 'y' ]; then
   buildPackages
fi

exit 0
# End





