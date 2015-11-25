#!/bin/sh
###############################################################################
#
# Test data setup script for all OSSIM repositories.
#
# The test data directory, specified by the env var OSSIM_DATA is
# syncronized against a master repository. The master data repository is
# assumed to be NFS-mounted at the mount point specified in the environment
# variable "OSSIM_DATA_REPOSITORY". The data will be rsynced to the local
# directory specified by "OSSIM_DATA" env var.
#
###############################################################################

echo; echo "Running setup.sh script from <$PWD>...";

echo "STATUS: Checking presence of env var OSSIM_DATA = <$OSSIM_DATA>...";
if [ -z $OSSIM_DATA ] || [ ! -d $OSSIM_DATA ] ; then
  echo "ERROR: Env var OSSIM_DATA must be defined and exist in order to syncronize against data repository.";
  exit 1;
fi

echo "STATUS: Checking access to data repository at <$OSSIM_DATA_REPOSITORY>...";
if [ -z $OSSIM_DATA_REPOSITORY ] || [ ! -d $OSSIM_DATA_REPOSITORY ] ; then
  echo "ERROR: Env var OSSIM_DATA_REPOSITORY must be defined and exist in order to syncronize against data repository.";
  exit 1;
fi

# rsync elevation data:
echo "STATUS: Syncing elevation data...";
rsync -rm --delete $OSSIM_DATA_REPOSITORY/elevation/dted/level0 $OSSIM_DATA/elevation/dted;
if [ $? != 0 ] ; then 
  echo "ERROR: Failed data repository rsync of elevation.";
  exit 1;
fi

# rsync nadcon data:
echo "STATUS: Syncing nadcon data...";
rsync -rm --delete $OSSIM_DATA_REPOSITORY/elevation/nadcon $OSSIM_DATA/elevation;
if [ $? != 0 ] ; then 
  echo "ERROR: Failed data repository rsync of nadcon grids.";
  exit 1;
fi

# rsync geoid 96 data:
echo "STATUS: Syncing geoid96 data...";
rsync -rm --delete $OSSIM_DATA_REPOSITORY/elevation/geoid96_little_endian/ $OSSIM_DATA/elevation/geoids/geoid96;
if [ $? != 0 ] ; then 
  echo "ERROR: Failed data repository rsync of geoid96 grids.";
  exit 1;
fi

# rsync geoid 99 data:
echo "STATUS: Syncing geoid99 data...";
rsync -rm --delete $OSSIM_DATA_REPOSITORY/elevation/geoid99_little_endian/ $OSSIM_DATA/elevation/geoids/geoid99;
if [ $? != 0 ] ; then 
  echo "ERROR: Failed data repository rsync of geoid99 grids.";
  exit 1;
fi

#rsync imagery
echo "STATUS: Syncing image data...";
rsync -rm --delete $OSSIM_DATA_REPOSITORY/test/data/public/ $OSSIM_DATA/ossim_data;
if [ $? != 0 ] ; then 
  echo "ERROR: Failed data repository rsync of imagery.";
  exit 1;
fi
  
exit 0;


