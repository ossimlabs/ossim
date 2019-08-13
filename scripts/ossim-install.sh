#!/bin/bash
ZIP_OPTION=$1
pushd `dirname ${BASH_SOURCE[0]}` >/dev/null
OSSIMCI_SCRIPT_DIR=`pwd -P`
pushd $OSSIMCI_SCRIPT_DIR/../../.. >/dev/null
export OSSIM_DEV_HOME=$PWD
popd > /dev/null
popd >/dev/null

source $OSSIMCI_SCRIPT_DIR/ossim-env.sh

# Establish CMAKE's install directory:
if [ -z "$OSSIM_INSTALL_PREFIX" ]; then
    export OSSIM_INSTALL_PREFIX=$OSSIM_DEV_HOME/install
fi

echo "STATUS: Checking presence of env var OSSIM_BUILD_DIR = <$OSSIM_BUILD_DIR>...";
if [ -z $OSSIM_BUILD_DIR ]; then
  export OSSIM_BUILD_DIR=$OSSIM_DEV_HOME/build;
  if [ ! -d $OSSIM_BUILD_DIR ] ; then
    echo "ERROR: OSSIM_BUILD_DIR = <$OSSIM_BUILD_DIR> does not exist at this location. Cannot install";
    exit 1;
  fi
fi

echo "STATUS: Performing make install to <$OSSIM_INSTALL_PREFIX>"
pushd $OSSIM_DEV_HOME/ossim/scripts
./install.sh
if [ $? -ne 0 ]; then
  echo; echo "ERROR: Error encountered ossim install. Check the console log and correct."
  popd >/dev/null
  exit 1
fi
popd >/dev/null

mkdir -p $OSSIM_INSTALL_PREFIX/etc/profile.d
mkdir -p $OSSIM_INSTALL_PREFIX/share/ossim/templates

cp $OSSIM_DEV_HOME/ossim/support/linux/etc/profile.d/ossim.sh $OSSIM_INSTALL_PREFIX/etc/profile.d/ossim.sh
cp $OSSIM_DEV_HOME/ossim/support/linux/etc/profile.d/ossim.csh $OSSIM_INSTALL_PREFIX/etc/profile.d/ossim.csh
cp $OSSIM_DEV_HOME/ossim/support/linux/service-wrapper-initd-template $OSSIM_INSTALL_PREFIX/share/ossim/templates/service-wrapper-initd-template
cp $OSSIM_DEV_HOME/ossim/support/linux/service-wrapper-systemd-template $OSSIM_INSTALL_PREFIX/share/ossim/templates/service-wrapper-systemd-template

chmod -R 755 $OSSIM_INSTALL_PREFIX/etc/profile.d
chmod 755 $OSSIM_INSTALL_PREFIX/share/ossim/templates/service-wrapper*

#install -p -m755 -D $OSSIM_DEV_HOME/ossim/support/linux/etc/profile.d/ossim.sh $OSSIM_INSTALL_PREFIX/etc/profile.d/ossim.sh
#install -p -m755 -D $OSSIM_DEV_HOME/ossim/support/linux/etc/profile.d/ossim.csh $OSSIM_INSTALL_PREFIX/etc/profile.d/ossim.csh
#install -p -m755 -D $OSSIM_DEV_HOME/ossim/support/linux/service-wrapper-initd-template $OSSIM_INSTALL_PREFIX/share/ossim/templates/service-wrapper-initd-template
#install -p -m755 -D $OSSIM_DEV_HOME/ossim/support/linux/service-wrapper-systemd-template $OSSIM_INSTALL_PREFIX/share/ossim/templates/service-wrapper-systemd-template

echo; echo "STATUS: Install completed successfully. Install located in $OSSIM_INSTALL_PREFIX"

if [ "$BUILD_KAKADU_PLUGIN" = "ON" ]; then
   if [ -z $KAKADU_LIBRARY ]; then
      echo "ERROR: KAKADU_LIBRARY env variable not set for install"
      exit 1
   fi

   if [ -z $KAKADU_AUX_LIBRARY ]; then
      echo "ERROR: KAKADU_AUX_LIBRARY env variable not set for install"
      exit 1
   fi

   # Need Kakadu shared libs for jpip server. drb - 20160414
   echo "STATUS: Performing install of $KAKADU_LIBRARY to <$OSSIM_INSTALL_PREFIX>"
   cp $KAKADU_LIBRARY $OSSIM_INSTALL_PREFIX/lib64
   echo "STATUS: Performing install of $KAKADU_AUX_LIBRARY to <$OSSIM_INSTALL_PREFIX>"
   cp $KAKADU_AUX_LIBRARY $OSSIM_INSTALL_PREFIX/lib64
fi

if [ "$BUILD_OMS" == "ON" ] ; then
   echo "STATUS: Performing joms install to <$OSSIM_INSTALL_PREFIX>"
   pushd $OSSIM_DEV_HOME/ossim-oms/joms/build_scripts/linux
   ./install.sh
   if [ $? -ne 0 ]; then
     echo; echo "ERROR: Error encountered during make install of joms. Check the console log and correct."
     popd
     exit 1
   fi
fi 
TIMESTAMP=`date +%Y-%m-%d-%H%M`

#####
# For binary install we will go ahead and define a service wrapper
# for the JPIP server.
#
mkdir -p ${OSSIM_INSTALL_PREFIX}/etc/init.d ${OSSIM_INSTALL_PREFIX}/lib/systemd/system
chmod 755 ${OSSIM_INSTALL_PREFIX}/etc/init.d
chmod 755 ${OSSIM_INSTALL_PREFIX}/lib/systemd/system

#install -d -m755 ${OSSIM_INSTALL_PREFIX}/etc/init.d
#install -d -m755 ${OSSIM_INSTALL_PREFIX}/lib/systemd/system
pushd $OSSIM_DEV_HOME/ossim/support/linux/
app=jpip-server
sed -e "s/{{program_name}}/${app}/g"  -e "s/{{program_user}}/omar/g" -e "s/{{program_group}}/omar/g" < service-wrapper-systemd-template >${OSSIM_INSTALL_PREFIX}/lib/systemd/system/${app}.service
sed -e "s/{{program_name}}/${app}/g"  -e "s/{{program_user}}/omar/g" -e "s/{{program_group}}/omar/g" < service-wrapper-initd-template >${OSSIM_INSTALL_PREFIX}/etc/init.d/${app}
popd

echo; echo "STATUS: Writing install info file to: <$OSSIM_INSTALL_PREFIX/gocd_install.info>..."
pushd $OSSIM_INSTALL_PREFIX
INSTALL_DIRNAME=${PWD##*/}
echo "
Build timestamp: $TIMESTAMP
Pipeline Name:   $GO_PIPELINE_NAME
Job Name:        $GO_JOB_NAME
" > gocd_install.info
cd ..

if [ "$ZIP_OPTION" == "-z" ]; then
  echo; echo "STATUS: Zipping up install directory: <$INSTALL_DIRNAME>..."
  FILENAME_TS="install_$GO_PIPELINE_NAME_$TIMESTAMP.tgz"
  tar cvfz $FILENAME_TS $INSTALL_DIRNAME
  if [ $? -ne 0 ]; then
    echo; echo "ERROR: Error encountered while zipping the install dir. Check the console log and correct."
    popd
    exit 1
  fi

  # Create a link that can be used as artifact of latest build/install. This will
  # overwrite previous sandbox's so only the latest is used for testing (standalone)
  # or generating expected results
  ln -s $FILENAME_TS "install.tgz"
  echo "STATUS: Successfully zipped install dir to <$PWD/$FILENAME_TS> and created link <$PWD/install.tgz>."
fi

popd # Out of dir containing install subdir
