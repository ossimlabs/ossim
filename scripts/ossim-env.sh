#!/bin/bash 
# Set Continuous Integration Environment:

if [ -z $OSSIMCI_SCRIPT_DIR ] ; then
   pushd $(dirname ${BASH_SOURCE[0]})
   export OSSIMCI_SCRIPT_DIR=`pwd -P`
   popd >/dev/null
fi
if [ -z $WORKSPACE ] ; then
   if [ -z $OSSIM_DEV_HOME ] ; then
      pushd $OSSIMCI_SCRIPT_DIR/../.. >/dev/null
      export OSSIM_DEV_HOME=$PWD
      popd > /dev/null
   fi

else
   export OSSIM_DEV_HOME=$WORKSPACE
fi

echo "OSSIMCI_SCRIPT_DIR = ${OSSIMCI_SCRIPT_DIR}"
if [ -f "${OSSIMCI_SCRIPT_DIR}/git-prompt.sh" ] ; then
   echo "SOURCING ${OSSIMCI_SCRIPT_DIR}/git-prompt.sh"
  source $OSSIMCI_SCRIPT_DIR/git-prompt.sh
fi

if [ -z $OSSIM_INSTALL_PREFIX ]; then
  export OSSIM_INSTALL_PREFIX=$OSSIM_DEV_HOME/install
fi

# Setup JAVA Home
#
# If not explicitly set then try to set.  Add more for other OS's
# this should work with OpenJDK installation.
#
if [ -z $JAVA_HOME ] ; then
  if [ -d "/usr/lib/jvm/java" ] ; then
    export JAVA_HOME="/usr/lib/jvm/java"
  elif [ -f "/usr/libexec/java_home" ] ; then
    export JAVA_HOME=`/usr/libexec/java_home`
  fi
fi

if [ -z $OSSIM_GIT_BRANCH ] ; then
   pushd $OSSIMCI_SCRIPT_DIR
   export OSSIM_GIT_BRANCH=`__git_ps1 "%s"`
   popd
fi

echo "CURRENT BRANCH = ${OSSIM_GIT_BRANCH}"

if [ -z $S3_DELIVERY_BUCKET ]; then
  export S3_DELIVERY_BUCKET="s3://o2-delivery/${OSSIM_GIT_BRANCH}"
fi

if [ -z $KAKADU_VERSION ] ; then
   # later need to add tests.  This is the last version
   # before they started using C++11 and that does not build on
   # gcc < 4.8 so CentOS6 and REL6 , ... uses gcc 4.6 I do believe.
  export KAKADU_VERSION="v7_5-01123C"
fi

# Version definitions
if [ -z $OSSIM_DEPENDENCY_VERSION ]; then
   export OSSIM_DEPENDENCY_VERSION=1.0.0
fi

# for packaging and general version number 
#
if [ -z $OSSIM_VERSION ] ; then
   export OSSIM_VERSION=1.9.0
fi

if [ -z $OSSIM_VERSION_TAG ] ; then
   if [ "${OSSIM_GIT_BRANCH}" == "dev" ] ; then
      export OSSIM_VERSION_TAG="SNAPSHOT"
   else 
       export OSSIM_VERSION_TAG="RELEASE"
   fi
fi

# for packaging and general version number 
#
if [ -z $O2_VERSION ] ; then
   export O2_VERSION=1.0.0
fi

# for packaging and general version number 
#
if [ -z $TLV_VERSION ] ; then
   export TLV_VERSION=1.0.0
fi

# For RPM packaging
#
if [ -z $OSSIM_BUILD_RELEASE ] ; then
   export OSSIM_BUILD_RELEASE=1
fi

# For RPM packaging
#
if [ -z $O2_BUILD_RELEASE ] ; then
   export O2_BUILD_RELEASE=1
fi

if [ -z $TLV_BUILD_RELEASE ] ; then
   export TLV_BUILD_RELEASE=1
fi

if [ -z $BUILD_OSSIM_APPS ] ; then
   export BUILD_OSSIM_APPS=ON
fi

if [ -z $BUILD_OSSIM_CURL_APPS ] ; then
   export BUILD_OSSIM_CURL_APPS=ON
fi

if [ -d $OSSIM_DEV_HOME/ossim-video ] ; then
   if [ -z $BUILD_OSSIM_VIDEO ] ; then
      export BUILD_OSSIM_VIDEO=ON
   fi
else
   export BUILD_OSSIM_VIDEO=ON
fi

if [ -d $OSSIM_DEV_HOME/ossim-oms ] ; then
   if [ -z $BUILD_OMS ] ; then
      export BUILD_OMS=ON
   fi
else
   export BUILD_OMS=OFF
fi

if [ -d $OSSIM_DEV_HOME/ossim-gui ] ; then
   if [ -z $BUILD_OSSIM_GUI ] ; then
      export BUILD_OSSIM_GUI=ON
   fi
else
   export BUILD_OSSIM_GUI=OFF
fi

if [ -d $OSSIM_DEV_HOME/ossim-planet ] ; then
   if [ -z $BUILD_OSSIM_PLANET ] ; then
      export BUILD_OSSIM_PLANET=ON
   fi
else
   export BUILD_OSSIM_PLANET=OFF
fi

if [ -d $OSSIM_DEV_HOME/ossim-wms ] ; then
   if [ -z $BUILD_OSSIM_WMS ] ; then
      export BUILD_OSSIM_WMS=ON
   fi
else
   export BUILD_OSSIM_WMS=OFF
fi


if [ -d $OSSIM_DEV_HOME/ossim-plugins ] ; then

   if [ -z $BUILD_CNES_PLUGIN ] ; then
      export BUILD_CNES_PLUGIN=ON
   fi

   if [ -z $BUILD_WEB_PLUGIN ] ; then
      export BUILD_WEB_PLUGIN=ON
   fi

   if [ -z $BUILD_SQLITE_PLUGIN ] ; then
      export BUILD_SQLITE_PLUGIN=ON
   fi

   if [ -z $BUILD_KAKADU_PLUGIN ] ; then
      export BUILD_KAKADU_PLUGIN=ON
   fi

   if [ -z $BUILD_KML_PLUGIN ] ; then
      export BUILD_KML_PLUGIN=ON
   fi

   if [ -z $BUILD_GDAL_PLUGIN ] ; then
      export BUILD_GDAL_PLUGIN=ON
   fi

   if [ -z $BUILD_AWS_PLUGIN ] ; then
      export BUILD_AWS_PLUGIN=ON
   fi
#   if [ -z $BUILD_HDF5_PLUGIN ] ; then
#      export BUILD_HDF5_PLUGIN=ON
#   fi

   if [ -z $BUILD_POTRACE_PLUGIN ] ; then
      export BUILD_POTRACE_PLUGIN=ON
   fi
   
   if [ -z $BUILD_FFTW3_PLUGIN ] ; then
      export BUILD_FFTW3_PLUGIN=ON
   fi

   if [ -z $BUILD_GEOPDF_PLUGIN ] ; then
   export BUILD_GEOPDF_PLUGIN=ON
   fi

   if [ -z $BUILD_OPENCV_PLUGIN ] ; then
      export BUILD_OPENCV_PLUGIN=OFF
   fi

   if [ -z $BUILD_OPENJPEG_PLUGIN ] ; then
      export BUILD_OPENJPEG_PLUGIN=ON
   fi

   if [ -z $BUILD_PNG_PLUGIN ] ; then
      export BUILD_PNG_PLUGIN=ON
   fi

   if [ -z $BUILD_JPEG12_PLUGIN ] ; then
      export BUILD_JPEG12_PLUGIN=ON
   fi

   if [ -z $BUILD_CSM_PLUGIN ] ; then
      export BUILD_CSM_PLUGIN=OFF
   fi

   if [ -z $BUILD_MSP_PLUGIN ] ; then
      export BUILD_MSP_PLUGIN=ON
   fi

   if [ -z $BUILD_ATP_PLUGIN ] ; then
      export BUILD_ATP_PLUGIN=ON
   fi

   if [ -z $BUILD_OSSIM_HDF5_SUPPORT ] ; then
      export BUILD_OSSIM_HDF5_SUPPORT=ON
   fi
   if [ -z $BUILD_OSSIM_HDF5_SUPPORT ] ; then
      export BUILD_OSSIM_HDF5_SUPPORT=ON
   fi
fi

if [ -z $OSSIM_BUILD_ADDITIONAL_DIRECTORIES ] ; then
   if [ -d $OSSIM_DEV_HOME/ossim-private/ossim-kakadu-jpip-server ]; then
     export OSSIM_BUILD_ADDITIONAL_DIRECTORIES=$OSSIM_DEV_HOME/ossim-private/ossim-kakadu-jpip-server
   fi
fi

if [ \( "${BUILD_KAKADU_PLUGIN}"="ON" \) -o \( -d "$OSSIM_DEV_HOME/ossim-private/ossim-kakadu-jpip-server" \) ] ; then

   if [ -d "${OSSIM_DEV_HOME}/kakadu-${KAKADU_VERSION}" ] ; then
      if [ -z $KAKADU_ROOT_SRC ] ; then
         export KAKADU_ROOT_SRC="${OSSIM_DEV_HOME}/kakadu-${KAKADU_VERSION}"
      fi
      if [ -d "${KAKADU_ROOT_SRC}/lib/Linux-x86-64-gcc" ] ; then
         if [ -z $KAKADU_LIBRARY ] ; then
              export KAKADU_LIBRARY="${KAKADU_ROOT_SRC}/lib/Linux-x86-64-gcc/libkdu_v75R.so"
         fi
         if [ -z $KAKADU_AUX_LIBRARY ] ; then
            export KAKADU_AUX_LIBRARY="${KAKADU_ROOT_SRC}/lib/Linux-x86-64-gcc/libkdu_a75R.so"
         fi
      fi
   fi
fi

# For OSSIM run-time environment:
if [ -z $OSSIM_DATA ] ; then
   export OSSIM_DATA="/data"
fi
if [ -z $OSSIM_BATCH_TEST_DATA ] ; then
   export OSSIM_BATCH_TEST_DATA="$OSSIM_DATA/ossim-data/${OSSIM_GIT_BRANCH}"
fi
if [ -z $OSSIM_BATCH_TEST_EXPECTED ] ; then
   export OSSIM_BATCH_TEST_EXPECTED="$OSSIM_DATA/ossim-expected/${OSSIM_GIT_BRANCH}"
fi
if [ -z $OSSIM_BATCH_TEST_RESULTS ] ; then
   export OSSIM_BATCH_TEST_RESULTS="$OSSIM_DATA/ossim-results/${OSSIM_GIT_BRANCH}"
fi
if [ -z $OSSIM_PREFS_FILE ] ; then
   if [ -f $OSSIM_INSTALL_PREFIX/ossim.config ] ; then
      export OSSIM_PREFS_FILE=$OSSIM_INSTALL_PREFIX/ossim.config
   elif [ -f $OSSIM_INSTALL_PREFIX/share/ossim/ossim-preferences-template ] ; then
      export OSSIM_PREFS_FILE=$OSSIM_INSTALL_PREFIX/share/ossim/ossim-preferences-template
   fi
fi

# For S3 storage/syncing of test data
if [ -z $S3_DATA_BUCKET ] ; then
   export S3_DATA_BUCKET="s3://o2_test_data"
fi


echo "S3_DATA_BUCKET = ${S3_DATA_BUCKET}"
echo "OSSIM_DATA = ${OSSIM_DATA}"
echo "OSSIM_BATCH_TEST_DATA = ${OSSIM_BATCH_TEST_DATA}"
echo "OSSIM_INSTALL_PREFIX = ${OSSIM_INSTALL_PREFIX}"
echo "OSSIM_PREFS_FILE = ${OSSIM_PREFS_FILE}"
