#!/bin/bash
####################################################################################################
#
#  This script is launched from inside the ossim-build-context container. It builds OSSIM libs and
#  executables as well as OFPGA libs and executables. The product files will be in the
# install temporary directory. They will be copied into the final ofpga-run-context docker image.
#
####################################################################################################

export BUILD_OSSIM_QT4=ON
export BUILD_OSSIM_FPGA=ON
export BUILD_PNG_PLUGIN=ON
export BUILD_CNES_PLUGIN=ON
export BUILD_GDAL_PLUGIN=ON
export OSSIM_DEV_HOME=$PWD
export OSSIM_BUILD_DIR=$PWD/build
export OSSIM_INSTALL_PREFIX=$PWD/install
export OFPGA_HOME=$PWD/ossim-fpga

echo "Running OSSIM cmake"
ossim/cmake/scripts/ossim-cmake-config.sh
if [ $? != 0 ]; then
  echo "Error in cmake config"ll
  exit 1
fi

echo "Running OSSIM make"
pushd $OSSIM_BUILD_DIR
make -j 8
if [ $? != 0 ]; then
  echo "Error encountered during make"
  exit 1
fi
echo "Running OSSIM install"
make install
if [ $? != 0 ]; then
  echo "Error encountered during make install"
  exit 1
fi
popd

pushd ossim-fpga
mkdir -p build
pushd build
cmake ..
if [ $? != 0 ]; then
  echo "Error encountered during ossim-fpga cmake"
  exit 1
fi
make
if [ $? != 0 ]; then
  echo "Error encountered during ossim-fpga make"
  exit 1
fi
echo "Copying fpga-ortho items to $OSSIM_INSTALL_PREFIX"
mv bin/* $OSSIM_INSTALL_PREFIX/bin
mv lib/* $OSSIM_INSTALL_PREFIX/lib
popd
popd

pushd fpga-ortho
mkdir -p build
pushd build
cmake ..
if [ $? != 0 ]; then
  echo "Error encountered during fpga-ortho cmake"
  exit 1
fi
make
if [ $? != 0 ]; then
  echo "Error encountered during fpga-ortho make"
  exit 1
fi
echo "Copying fpga-ortho items to $OSSIM_INSTALL_PREFIX"
mv bin/* $OSSIM_INSTALL_PREFIX/bin
mv lib/* $OSSIM_INSTALL_PREFIX/lib
popd
popd

#pushd ds-client-app
#mkdir -p build
#pushd build
#echo "Running client_app cmake/make"
#cmake ..
#if [ $? != 0 ]; then
#  echo "Error encountered during ds-client-app cmake"
#  exit 1
#fi
#make
#if [ $? != 0 ]; then
#  echo "Error encountered during ds-client-app make"
#  exit 1
#fi
#echo "Copying client_app executable to $OSSIM_INSTALL_PREFIX/bin"
#mv client_app $OSSIM_INSTALL_PREFIX/bin
#popd
#rm -rf build
#popd

echo "Done."
exit 0
