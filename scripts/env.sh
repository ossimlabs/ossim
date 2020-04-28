#!/bin/bash 
pushd `dirname ${BASH_SOURCE[0]}` >/dev/null
export ENV_SCRIPT_DIR=`pwd -P`
popd >/dev/null

. $ENV_SCRIPT_DIR/git-prompt.sh

if [ -z $OSSIM_GIT_BRANCH ] ; then
  export OSSIM_GIT_BRANCH=`__git_ps1 "%s"`
fi

if [ -z $WORKSPACE ] ; then
   if [ -z "$OSSIM_DEV_HOME" ]; then
      pushd $ENV_SCRIPT_DIR/../.. >/dev/null
      export OSSIM_DEV_HOME=$PWD
      popd >/dev/null
   fi
else
   if [ "$OSSIM_DEV_HOME" == "" ] ; then
      export OSSIM_DEV_HOME=$WORKSPACE
   fi
fi

if [ -z "$OSSIM_MAKE_JOBS" ]; then
   export OSSIM_MAKE_JOBS=4
fi

if [ -z "$OSSIM_INSTALL_PREFIX" ]; then
   export OSSIM_INSTALL_PREFIX=$OSSIM_DEV_HOME/install
fi
if [ -z "$OSSIM_BUILD_DIR" ]; then
   export OSSIM_BUILD_DIR=$OSSIM_DEV_HOME/build
fi
export CMAKE_CONFIG_SCRIPT=$OSSIM_DEV_HOME/ossim/cmake/scripts/ossim-cmake-config.sh


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
# For RPM packaging
#
if [ -z $OSSIM_BUILD_RELEASE ] ; then
   export OSSIM_BUILD_RELEASE=1
fi

if [ -z BUILD_OSSIM_MPI_SUPPORT ] ; then
  export BUILD_OSSIM_MPI_SUPPORT=OFF
fi

if [ -z $BUILD_OSSIM_APPS ] ; then
   export BUILD_OSSIM_APPS=ON
fi

if [ -z $BUILD_OSSIM_CURL_APPS ] ; then
   export BUILD_OSSIM_CURL_APPS=OFF
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

if [ -d $OSSIM_DEV_HOME/ossim-qt4 ] ; then
   if [ -z $BUILD_OSSIM_QT4 ] ; then
      export BUILD_OSSIM_QT4=ON
   fi
else
   export BUILD_OSSIM_QT4=OFF
fi

if [ -d $OSSIM_DEV_HOME/ossim-plugins ] ; then

   if [ -z $BUILD_CNES_PLUGIN ] ; then
      export BUILD_CNES_PLUGIN=ON
   fi

   if [ -z $BUILD_CSM_PLUGIN ] ; then
      export BUILD_CSM_PLUGIN=OFF
   fi

   if [ -z $BUILD_WEB_PLUGIN ] ; then
      export BUILD_WEB_PLUGIN=OFF
   fi

   if [ -z $BUILD_SQLITE_PLUGIN ] ; then
      export BUILD_SQLITE_PLUGIN=OFF
   fi

   if [ -z $BUILD_KAKADU_PLUGIN ] ; then
      export BUILD_KAKADU_PLUGIN=OFF
   fi

   if [ -z $BUILD_KML_PLUGIN ] ; then
      export BUILD_KML_PLUGIN=OFF
   fi

   if [ -z $BUILD_GDAL_PLUGIN ] ; then
      export BUILD_GDAL_PLUGIN=OFF
   fi

   #if [ -z $BUILD_HDF5_PLUGIN ] ; then
   #   export BUILD_HDF5_PLUGIN=ON
   #fi

   if [ -z $BUILD_POTRACE_PLUGIN ] ; then
      export BUILD_POTRACE_PLUGIN=OFF
   fi
   
   if [ -z $BUILD_FFTW3_PLUGIN ] ; then
      export BUILD_FFTW3_PLUGIN=OFF
   fi

   if [ -z $BUILD_GEOPDF_PLUGIN ] ; then
   export BUILD_GEOPDF_PLUGIN=OFF
   fi

   if [ -z $BUILD_OPENCV_PLUGIN ] ; then
      export BUILD_OPENCV_PLUGIN=OFF
   fi

   if [ -z $BUILD_OPENJPEG_PLUGIN ] ; then
      export BUILD_OPENJPEG_PLUGIN=OFF
   fi

   if [ -z $BUILD_PNG_PLUGIN ] ; then
      export BUILD_PNG_PLUGIN=OFF
   fi

   if [ -z $BUILD_JPEG12_PLUGIN ] ; then
      export BUILD_JPEG12_PLUGIN=ON
   fi

   if [ -z $BUILD_OSSIM_HDF5_SUPPORT ] ; then
      export BUILD_OSSIM_HDF5_SUPPORT=OFF
   fi
fi

# if [ -z $OSSIM_BUILD_ADDITIONAL_DIRECTORIES ] ; then
#    if [ -d $OSSIM_DEV_HOME/ossim-private/ossim-kakadu-jpip-server ]; then
   #   export OSSIM_BUILD_ADDITIONAL_DIRECTORIES=$OSSIM_DEV_HOME/ossim-private/ossim-kakadu-jpip-server
   # fi
# fi

if [ "${BUILD_KAKADU_PLUGIN}"="ON"  ] ; then
# if [ \( "${BUILD_KAKADU_PLUGIN}"="ON" \) -o \( -d "$OSSIM_DEV_HOME/ossim-private/ossim-kakadu-jpip-server" \) ] ; then

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

