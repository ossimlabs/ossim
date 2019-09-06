#!/bin/bash
####################################################################################################
#
#  This script ultimately builds the ossim-context docker image for executing
#  available OSSIM applications.
#
#  Usage:  build-ossim-image.sh  [-x]
#
#  If the -x option is specified, the building of ossim-context image and clean-up is skipped.
#
#  The following are the steps performed:
#
#  1. Create the ossim-context image. This docker image contains all OSSIM dependency SDKs
#     and build tools (gcc, cmake, make, etc.) needed for building the OSSIM library, plugins
#     and executables, as well as the FPGA library and executables needed for orthorectification.
#
#  2. In anticipation of building the final ortho-run-context image, all the necessary repos,
#     directories, and files are copied to the current directory so that they are available to
#     the docker build environment. It is necessary to be on the DirectStream VPN to access
#     the FPGA component code.
#
#  3. The ossim-context image is run, and the build-ortho.sh script is run inside the
#     container. This script uses the ossim build context to compile and link the OSSIM libraries,
#     executables, and OFPGA library and executables. The product binaries are saved to a local
#     temporary install direcory.
#
#  4. Build the ortho-run-context image. The image is based on the ossim-context image. It
#     copies all binaries and other needed data from the local file system into the container image.
#
#  5. Save the ortho-run-context image to a tar file and delete all temporary directories, repos,
#     and files that were created for the build.
#
#  Author: Oscar Kramer, oscar.kramer@maxar.com
#
####################################################################################################
# Uncomment following line to debug script line by line:
#set -x; trap read debug

DO_CLEANUP=true
if [ "$1" == "-x" ] ; then
   DO_CLEANUP=false;
fi

# Make sure we start execution at the script directory:
pushd `dirname ${BASH_SOURCE[0]}`

if [ "$DO_CLEANUP" == "true" ]; then
   echo; echo "### Deleting any running ossim-context image and container. ###"; echo
   docker rm ossim-context
   docker rmi ossim-context:latest
   pushd docker/ossim-context
   echo; echo "### Building ossim-context-image. ###"; echo
   docker build --force-rm -t ossim-context .
   if [ $? != 0 ]; then
     echo "Error building ossim-context image"
     popd; exit 1
   fi
   popd  # back to script dir
else
   echo; echo "Skipping cleanup"
fi

# Build the docker image that will contain all OFPGA and client_app applications and scripts.
echo; echo "### Deleting any running ortho-run-context image and container. ###"; echo
docker rmi ossim-context:latest

# Bring in all needed repos if not already here:
if [ ! -d "ossim" ]; then  # was clean-up done?
   echo; echo "### Cloning all needed repositories ###"; echo
   git clone --branch dev --single-branch git@github.com:ossimlabs/ossim.git
   git clone --branch dev --single-branch git@github.com:ossimlabs/ossim-plugins.git

   # The final container will need some data items so copy them here:
   echo; echo "### Copying elevation data to current directory. ###"; echo
   cp -R $OSSIM_DATA/elevation/dted/level0 .
   cp -R $OSSIM_DATA/elevation/geoids .
fi

popd  # Back to original working directory

echo; echo "### Saving ossim-context image to tar file to $PWD. ###"; echo
docker save -o ossim-context.tar ossim-context:latest
if [ $? != 0 ]; then
  echo "Error saving ossim-context to tar file."
  exit 1
fi

echo; echo "### Success ###"; echo
