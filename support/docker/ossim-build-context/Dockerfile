# ossim-build-context image
# Provides build and runtime context for OSSIM-based application builds
#
# How to use:
#   docker build --rm -t ossim-build-context .

FROM library/centos:latest

USER root

COPY ossim.repo /etc/yum.repos.d/ossim.repo

RUN yum -y install epel-release

RUN yum clean all

RUN yum -y install git gcc geos-devel gcc-c++ \
   openssl-devel curl-devel libgeotiff-devel libtiff-devel libjpeg-devel libpng-devel OpenThreads-devel \
   libgif-devel cmake make jsoncpp-devel gdal-devel freetype-devel ffmpeg-devel qt4-devel

