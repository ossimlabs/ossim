#!/bin/bash

#
# Example build confugre used on Ubuntu by Ed Leaver.
#
# Use this as an example to show how to use 
# cmake to configure the parts of OSSIM.
#
BUILD_TYPE=Release
INSTALL_PREFIX=/usr/local/ossim-dev

OS=`cat /etc/os-release | grep -e "^ID=" | cut -d'=' -f2 | sed 's/"//g'`
OS_VERSION=`cat /etc/os-release | grep -e "^VERSION_ID=" | cut -d'=' -f2 | sed 's/"//g'`
OSV=$OS$OS_VERSION

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
BUILDDIR="$SCRIPTDIR/Build-$OSV-$BUILD_TYPE"
SRCDIR="$SCRIPTDIR/ossim"

echo "SRC:   $SRCDIR"
echo "BUILD: $BUILDDIR"

CMAKE=cmake

if [ "$OSV" = "ubuntu18.04" ] ; then
   MPIINC=/usr/include/mpich
   MPIDIR=/usr/lib/x86_64-linux-gnu/
   MPI_C_COMPILER=/usr/bin/mpicc.mpich
   MPI_CXX_COMPILER=/usr/bin/mpicxx.mpich
   MPI_mpi_LIBRARY=$MPIDIR/lib/libmpich.so
   MPI_mpi_cxx_LIBRARY=$MPIDIR/lib/libmpichcxx.so
   MPIEXEC_EXECUTABLE=/usr/bin/mpirun.mpich
else
   MPIINC=/usr/include/openmpi-x86_64
   MPIDIR=/usr/lib64/openmpi
   MPI_C_COMPILER=$MPIDIR/bin/mpicc
   MPI_CXX_COMPILER=$MPIDIR/bin/mpicxx
   MPI_mpi_LIBRARY=$MPIDIR/lib/libmpi.so
   MPI_mpi_cxx_LIBRARY=$MPIDIR/lib/libmpi_cxx.so
   MPIEXEC_EXECUTABLE=$MPIDIR/bin/mpirun
fi
MPI_C_HEADER_DIR=$MPIINC
MPI_CXX_HEADER_DIR=$MPIINC

OPTIONS="  -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX "
OPTIONS+=" -DCMAKE_BUILD_TYPE=$BUILD_TYPE "

if [ "$OSV" = "fedora32" ] ; then
    OPTIONS+=" -DMPI_C_HEADER_DIR=$MPI_C_HEADER_DIR "
    OPTIONS+=" -DMPI_CXX_HEADER_DIR=$MPI_CXX_HEADER_DIR "
else
    OPTIONS+=" -DMPI_C_INCLUDE_PATH=$MPI_C_HEADER_DIR "
    OPTIONS+=" -DMPI_CXX_INCLUDE_PATH=$MPI_CXX_HEADER_DIR "
fi

if [ "$OSV" = "centos7" ] ; then
    OPTIONS+=" -DBUILD_OSSIM_HDF5_SUPPORT=OFF "
else
    OPTIONS+=" -DBUILD_OSSIM_HDF5_SUPPORT=ON "
fi

if [ "$OSV" = "ubuntu18.04" ] ; then
    OPTIONS+=" -DHDF5A_INCLUDE_DIR=/usr/include/hdf5/serial "
    OPTIONS+=" -DHDF5A_CPP_LIB=/usr/lib/x86_64-linux-gnu/libhdf5_cpp.so "
    OPTIONS+=" -DHDF5A_LIB=/usr/lib/x86_64-linux-gnu/libhdf5_serial.so "
    OPTIONS+=" -DGEOS_INCLUDE_DIR=/usr/local/geos-3.8.2/include "
    OPTIONS+=" -DGEOS_C_LIB=/usr/local/geos-3.8.2/lib/libgeos_c.so.1 "
    OPTIONS+=" -DGEOS_LIB=/usr/local/geos-3.8.2/lib/libgeos.so.3.8.2 "
fi

OPTIONS+=" -DMPI_C_COMPILER=$MPI_C_COMPILER "
OPTIONS+=" -DMPI_CXX_COMPILER=$MPI_CXX_COMPILER "
OPTIONS+=" -DMPIEXEC_EXECUTABLE=$MPIEXEC_EXECUTABLE "
OPTIONS+=" -DBUILD_OSSIM_MPI_SUPPORT:BOOL=ON "

OPTIONS+=" -DBUILD_OSSIM_GUI=ON "
OPTIONS+=" -DINSTALL_DOC=ON "
OPTIONS+=" -DBUILD_OMS:BOOL=ON "
OPTIONS+=" -DBUILD_OSSIM_APPS:BOOL=ON "
OPTIONS+=" -DBUILD_OSSIM_CURL_APPS:BOOL=OFF "
OPTIONS+=" -DBUILD_OSSIM_FRAMEWORKS:BOOL=ON "
OPTIONS+=" -DBUILD_OSSIM_FREETYPE_SUPPORT:BOOL=ON "
OPTIONS+=" -DBUILD_OSSIM_ID_SUPPORT:BOOL=ON "
OPTIONS+=" -DBUILD_OSSIM_PLANET:BOOL=ON "
OPTIONS+=" -DBUILD_OSSIM_TESTS:BOOL=ON "
OPTIONS+=" -DBUILD_OSSIM_VIDEO:BOOL=ON "
OPTIONS+=" -DBUILD_OSSIM_WMS:BOOL=ON "
OPTIONS+=" -DBUILD_SHARED_LIBS:BOOL=ON "

mkdir -p $BUILDDIR; cd $BUILDDIR
$CMAKE  $OPTIONS -B "$BUILDDIR" -S "$SRCDIR"
