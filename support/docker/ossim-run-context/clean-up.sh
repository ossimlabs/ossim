#!/bin/bash
####################################################################################################
#
# The docker build image command requires all files and subdirs to be located in the same directory
# as the Dockerfile, so the build scipt (../../build-docker-image.sh) copies and clones many items
# into this directory before building the image. This script deletes all copied items. It is
# usually run as part of the build script but can be run standalone as root.
#
####################################################################################################

DELETION_LIST=(geoids \
               level0 \
               ossim-fpga \
               fpga-ortho \
               ossim \
               ossim-plugins \
               ossim-qt4 \
               runDsOrtho.sh \
               ds-client-app \
               install \
               build)

for f in ${DELETION_LIST[@]}; do
   if [ -d "$f" ]; then
      rm -rf $f
   elif [ -f "$f" ]; then
      rm $f
   fi
done
