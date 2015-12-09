SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
pushd $SCRIPT_DIR/../../..
OSSIM_DEV_HOME=$PWD
echo "STATUS: Setting OSSIM_DEV_HOME = <$OSSIM_DEV_HOME>"

echo "STATUS: Checking presence of env var OSSIM_BUILD_DIR = <$OSSIM_BUILD_DIR>...";
if [ -z $OSSIM_BUILD_DIR ]; then
  OSSIM_BUILD_DIR=$OSSIM_DEV_HOME/build;
  if [ ! -d $OSSIM_BUILD_DIR ] ; then
    echo "ERROR: OSSIM_BUILD_DIR = <$OSSIM_BUILD_DIR> does not exist at this location. Cannot install";
    exit 1;
  fi
fi

if [ -z $OSSIM_INSTALL_DIR ]; then
  OSSIM_INSTALL_DIR=$OSSIM_DEV_HOME/install;
  echo "INFO: OSSIM_INSTALL_DIR environment variable is not defined. Defaulting to <$OSSIM_INSTALL_DIR>";
fi

if [ ! -d $OSSIM_INSTALL_DIR ] ; then
  echo "INFO: Installation directory <$OSSIM_INSTALL_DIR> does not exist. Creating...";
  mkdir $OSSIM_INSTALL_PREFIX;
fi

pushd $OSSIM_BUILD_DIR
echo "STATUS: Performing make install to <$OSSIM_INSTALL_DIR>"
make install
if [ $? -ne 0 ]; then
  echo; echo "ERROR: Error encountered during make install. Check the console log and correct."
  popd
  exit 1
fi
echo; echo "STATUS: Install completed successfully. Install located in $OSSIM_INSTALL_DIR"
popd # out of OSSIM_BUILD_DIR

popd # out of OSSIM_DEV_HOME
exit 0

