#!/bin/bash
####################################################################################################
#
#  Script for launching ossim-context docker image. See usage function.
#
#  Author: Oscar Kramer, oscar.kramer@radiantsolutions.com
#
####################################################################################################
OSSIM_CONTEXT_IMAGE="ossim-context"

usage() {
  echo
  echo "Launches docker container from ossim-context image. If there is no image loaded in the"
  echo "docker registry, the script will look for '${OSSIM_CONTEXT_IMAGE}.tar' in the current directory"
  echo "and load it."
  echo
  echo " Usage:  docker-run.sh [-x] [<command> [<args>]]"
  echo
  echo "Options: "
  echo "   -x          No mounts are performed. The default is to mount the current working"
  echo "               directory on /home/ofpga in the container."
  echo "   -h, --help  Prints this usage."
  echo "   <command>   If a command is specified, it will be executed in the container accompanied."
  echo "               by any <args> provided. Otherwise, the container is entered in interactive "
  echo "               mode."
  echo
}

if [[ $1 == "-h" || $1 == "--help" ]] ; then
   usage
   exit 0
fi

# Look for docker image:
X=`docker images | grep $OSSIM_CONTEXT_IMAGE`
if [[ -z $X ]] ; then
  if [[ ! -e "${OSSIM_CONTEXT_IMAGE}.tar" ]]; then
     echo "The $OSSIM_CONTEXT_IMAGE image has not been loaded and the archive was not found in $PWD"
     exit 1
  fi
  echo "Loading '$PWD/${OSSIM_CONTEXT_IMAGE}.tar' archive image"
  docker load -i ${OSSIM_CONTEXT_IMAGE}.tar
  if [[ $? != 0 ]]; then
     echo "Error encountered loading docker image $PWD/${OSSIM_CONTEXT_IMAGE}.tar"
     exit 1
  fi
fi

if [ "$1" == "-x" ] ; then
  # No mounts:
  shift
  docker run --rm -it -w /home/ossim ossim-context:latest $*

else
  # With mounts:
  docker run -it --rm -w /home/ossim \
             --net=host --ipc host --env="DISPLAY" \
             --volume="$HOME/.Xauthority:/root/.Xauthority:rw" \
             --mount type=bind,source=$PWD,target=/home/ossim \
             ossim-context:latest $*
fi

