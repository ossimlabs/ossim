#/bin/bash

apt-get -y update; apt-get -y install curl \
    zip \
    unzip \
    snapd \
    gdal-bin \
    gnupg software-properties-common \
    qgis qgis-plugin-grass \
    postgresql-17 \
    postgresql-17-postgis-3 \ 
    git \
    python3 \
    pipx \
    docker.io \
    python3-libtiff \
    libgeos++-dev \
    cmake \
    libcurl4-openssl-dev \
    libsqlite3-dev \
    libgeotiff-dev \
    vim \
    tree

