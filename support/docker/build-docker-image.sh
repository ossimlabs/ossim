#!/bin/bash
####################################################################################################
#
#  This script ultimately builds the ortho-run-context docker image for executing the OFPGA
#  utilities and available OSSIM applications.
#
#  Usage:  build-docker-image.sh  [-x]
#
#  If the -x option is specified, the building of ossim-build-context image and clean-up is skipped.
#
#  The following are the steps performed:
#
#  1. Create the ossim-build-context image. This docker image contains all OSSIM dependency SDKs
#     and build tools (gcc, cmake, make, etc.) needed for building the OSSIM library, plugins
#     and executables, as well as the FPGA library and executables needed for orthorectification.
#
#  2. In anticipation of building the final ortho-run-context image, all the necessary repos,
#     directories, and files are copied to the current directory so that they are available to
#     the docker build environment. It is necessary to be on the DirectStream VPN to access
#     the FPGA component code.
#
#  3. The ossim-build-context image is run, and the build-ortho.sh script is run inside the
#     container. This script uses the ossim build context to compile and link the OSSIM libraries,
#     executables, and OFPGA library and executables. The product binaries are saved to a local
#     temporary install direcory.
#
#  4. Build the ortho-run-context image. The image is based on the ossim-build-context image. It
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
   # Build the docker image that will serve as the base for the final ortho-context-image.
   # This image will contain all OSSIM dependencies and build tools needed to build OFPGA apps:
   echo; echo "### Deleting any running ossim-build-context image and container. ###"; echo
   docker rm ossim-build-context
   docker rmi ossim-build-context:latest
   pushd docker/ossim-build-context
   echo; echo "### Building ossim-build-context-image. ###"; echo
   docker build --force-rm -t ossim-build-context .
   if [ $? != 0 ]; then
     echo "Error building ossim-build-context image"
     popd; exit 1
   fi
   popd  # back to script dir
else
   echo; echo "Skipping cleanup"
fi

# Build the docker image that will contain all OFPGA and client_app applications and scripts.
echo; echo "### Deleting any running ortho-run-context image and container. ###"; echo
docker rmi ortho-run-context:latest
pushd docker/ortho-run-context

# Bring in all needed repos if not already here:
if [ ! -d "ossim" ]; then  # was clean-up done?
   echo; echo "### Cloning all needed repositories ###"; echo
   git clone --branch dev --single-branch git@github.com:ossimlabs/ossim.git
   git clone --branch dev --single-branch git@github.com:ossimlabs/ossim-plugins.git
   git clone --branch master --single-branch git@github.com:ossimlabs/ossim-qt4.git
   git clone --branch dev --single-branch git@github.com:Maxar-Corp/ossim-fpga.git

#   git clone --branch master --single-branch git@github.com:Maxar-Corp/ds-client-app.git
#   cp $HOME/bin/client_app .

   git clone --branch dev --single-branch git@github.com:Maxar-Corp/fpga-ortho.git
   pushd fpga-ortho/ds-gov-001
   rm Ingress Egress ortho_common_v1 GeographicProjection ElevationFinder rpc_worldToImage
   git clone --branch master --single-branch git@elements.ds-gov-001.directstream.com:ds-gov-001/Ingress.git
   git clone --branch master --single-branch git@elements.ds-gov-001.directstream.com:ds-gov-001/Egress.git
   git clone --branch master --single-branch git@elements.ds-gov-001.directstream.com:ds-gov-001/ortho_common_v1.git
   git clone --branch master --single-branch git@elements.ds-gov-001.directstream.com:ds-gov-001/GeographicProjection.git
   git clone --branch master --single-branch git@elements.ds-gov-001.directstream.com:ds-gov-001/ElevationFinder.git
   git clone --branch master --single-branch git@elements.ds-gov-001.directstream.com:ds-gov-001/rpc_worldToImage.git
   popd   # back to docker/ortho-run-context

   # The final container will need some data items so copy them here:
   echo; echo "### Copying elevation data to current directory. ###"; echo
   cp -R $OSSIM_DATA/elevation/dted/level0 .
   cp -R $OSSIM_DATA/elevation/geoids .

   # We'll also need the batch orthorectification script:
   echo; echo "### Copying orthorectification script. ###"; echo
   cp ../../runDsOrtho.sh .
fi

# Build OSSIM and client_app inside the ossim-build-context container
echo; echo "### Building libs and applications in ossim-build-context container. ###"; echo
docker run --rm -w /home/ofpga \
   --mount type=bind,source=$PWD,target=/home/ofpga \
   --mount type=bind,source=/data,target=/data \
   ossim-build-context:latest ./build-ortho.sh
if [ $? != 0 ]; then
  echo "Error building ortho in ossim-build-context container."
  popd ; exit 1
fi

# Now build the ortho-context image that will contain all OFPGA apps:
echo; echo "### Building ortho-run-context image. ###"; echo
docker build --force-rm -t ortho-run-context .
if [ $? != 0 ]; then
  echo "Error building ortho-run-context."
  popd ; exit 1
fi

if $DO_CLEANUP; then
   echo; echo "### Deleting temporary files and repos. ###"; echo
   docker run --rm -w /home/ofpga \
      --mount type=bind,source=$PWD,target=/home/ofpga \
      ossim-build-context:latest ./clean-up.sh
   if [ $? != 0 ]; then
     echo "Error cleaning up docker temporary directories via ossim-build-context container."
     popd ; exit 1
   fi
fi
popd  # Back to script dir
popd  # Back to original working directory

echo; echo "### Saving ortho-run-context image to tar file to $PWD. ###"; echo
docker save -o ortho-run-context.tar ortho-run-context:latest
if [ $? != 0 ]; then
  echo "Error saving ortho-run-context to tar file."
  popd ; exit 1
fi

echo; echo "### Success ###"; echo
