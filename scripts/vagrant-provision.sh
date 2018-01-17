#!/usr/bin/env bash

# This script is for use by Vagrant for building and installing ossim from source on Ubuntu 16.04.
# This script assumes the project root is synced into /home/vagrant/ossimlabs/ossim using the VagrantFile.
echo "Starting Vagrant provisioning script."
echo "Running as $USER"

setBashrcEnvVariable() {
    name=$1
    value=$2
    echo "$name=$value" >> $HOME/.bashrc
    source $HOME/.bashrc
}

installPackages() {
    sudo apt -y update
    sudo apt -y upgrade
    sudo apt -y install git libtiff5-dev libopenthreads-dev libjpeg-dev libgeos++-dev make cmake g++ libgeotiff-dev libjsoncpp-dev libav-tools
}

cloneSources() {
    mkdir -p $HOME/ossimlabs
    sudo chown vagrant:vagrant $HOME/ossimlabs/ # Owning in case file sync has the directory owned by root.

    cd $HOME/ossimlabs/
    # We assume ossim is already synced into the ossimlabs directory.
    git clone https://github.com/ossimlabs/omar-common
    # OMS and ossim-video not included as they break the build.
}

setBuildVariables() {
    setBashrcEnvVariable "OMAR_COMMON_PROPERTIES" "$HOME/ossimlabs/omar-common/omar-common-properties.gradle"
    setBashrcEnvVariable "BUILD_OSSIM_VIDEO" "OFF" # Turned off since FFMPEG fails the build.
    setBashrcEnvVariable "BUILD_OMS" "OFF" # Turned off since OMS fails the build (ossimPredator issue).
}

buildOssim() {
    echo "1" | $HOME/ossimlabs/ossim/scripts/build.sh # Echo "1" to choose 'release' option on first prompt.
    cd $HOME/ossimlabs/build/
    make
    setBashrcEnvVariable "PATH" "\"$PATH:$HOME/ossimlabs/build/bin/\""
}

installPackages
cloneSources
setBuildVariables
buildOssim

echo "Finished Vagrant provisioning script."