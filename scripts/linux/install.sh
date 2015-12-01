SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
pushd $SCRIPT_DIR/../../..
OSSIM_DEV_HOME=$PWD
echo "@@@@@ OSSIM_DEV_HOME=$OSSIM_DEV_HOME"

echo "STATUS: Checking presence of env var OSSIM_DATA = <$OSSIM_BUILD_DIR>...";
if [ -z $OSSIM_BUILD_DIR ] || [ ! -d $OSSIM_BUILD_DIR ] ; then
  echo "ERROR: Env var OSSIM_BUILD_DIR must be defined and exist in order to install the built binaries.";
  exit 1;
fi

if [ -z $OSSIM_INSTALL_PREFIX ]; then
  OSSIM_INSTALL_PREFIX=$OSSIM_DEV_HOME/install;
  echo "INFO: OSSIM_INSTALL_PREFIX environment variable is not defined. Defaulting to <$OSSIM_INSTALL_PREFIX>";
fi

if [ ! -d $OSSIM_INSTALL_PREFIX ] ; then
  echo "INFO: Installation directory <$OSSIM_INSTALL_PREFIX> does not exist. Creating...";
  mkdir $OSSIM_INSTALL_PREFIX;
fi

pushd $OSSIM_BUILD_DIR
echo "STATUS: Performing make install to <$OSSIM_INSTALL_PREFIX>"
make install
if [ $? -ne 0 ]; then
  echo; echo "ERROR: Error encountered during make install. Check the console log and correct."
  popd
  exit 1
fi
echo; echo "STATUS: Install completed successfully. Install located in $OSSIM_INSTALL_PREFIX"
popd # out of OSSIM_BUILD_DIR

popd # out of OSSIM_DEV_HOME
exit 0

