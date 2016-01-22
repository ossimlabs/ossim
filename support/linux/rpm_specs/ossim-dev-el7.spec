#---
# Example usage:
# rpmbuild -ba --define 'RPM_OSSIM_VERSION 1.9.0' --define 'BUILD_RELEASE 1' ossimlabs.spec
#---
Name:           ossim
Version:        %{RPM_OSSIM_VERSION} 
Release:        %{BUILD_RELEASE}%{?dist}
Summary:        Open Source Software Image Map library and command line applications
Group:          System Environment/Libraries
#TODO: Which version?
License:        LGPLv2+
URL:            https://github.com/orgs/ossimlabs/dashboard
Source0:        http://download.osgeo.org/ossim/source/%{name}-%{version}.tar.gz

BuildRequires: ant
BuildRequires: cmake
BuildRequires: ffmpeg-devel
BuildRequires: gdal-devel
BuildRequires: geos-devel
# BuildRequires: hdf4-devel
BuildRequires: hdf5a-devel
BuildRequires: java-devel
BuildRequires: libcurl-devel
BuildRequires: libgeotiff-devel
BuildRequires: libjpeg-devel
BuildRequires: libpng-devel
BuildRequires: libtiff-devel
#BuildRequires: libRaw-devel
BuildRequires: minizip-devel
BuildRequires: opencv-devel
BuildRequires: OpenSceneGraph-devel
BuildRequires: OpenThreads-devel
BuildRequires: podofo-devel
BuildRequires: qt4-devel
BuildRequires: sqlite-devel
BuildRequires: gpstk-devel
#BuildRequires: swig

%description
OSSIM is a powerful suite of geospatial libraries and applications
used to process remote sensing imagery, maps, terrain, and vector data.

%package 	    devel
Summary:        Develelopment files for ossim
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
Development files for ossim.

%package 	    gdal-plugin
Summary:        GDAL ossim plugin
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description gdal-plugin
This sub-package contains the gdal ossim plugin for reading/writing images
supported by the gdal library.

%package 	    geocell
Summary:        Desktop electronic light table
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description geocell
Desktop electronic light table for geospatial image processing. Has 2D, 2 1/2D
and 3D viewer with image chain editing capabilities.

%package 	    geopdf-plugin
Summary:        geopdf ossim plugin
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description geopdf-plugin
This sub-package contains the geopdf ossim plugin for reading geopdf files via
the podofo library.

%if 0
%package 	    hdf-plugin
Summary:        HDF ossim plugin
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description hdf-plugin
This sub-package contains the Hierarchical Data Format(hdf) ossim plugin for
reading hdf4 and hdf5 images via the hdf4 and hdf5 libraries.
%endif

%if 0
%package 	    hdf4-plugin
Summary:        HDF4 ossim plugin
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description hdf4-plugin
This sub-package contains the Hierarchical Data Format(hdf) ossim plugin for
reading hdf4 images via the hdf4 libraries.
%endif

%if 1
%package 	    hdf5-plugin
Summary:        HDF5 ossim plugin
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description hdf5-plugin
This sub-package contains the Hierarchical Data Format(hdf) ossim plugin for
reading hdf5 images via the hdf5 libraries.
%endif

%package  	    kmlsuperoverlay-plugin
Summary:        kmlsuperoverlay ossim plugin
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description kmlsuperoverlay-plugin
This sub-package contains the kmlsuperoverlay ossim plugin for reading/writing
kml super overlays.

%package  	    las-plugin
Summary:        LAS ossim plugin
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description las-plugin
This sub-package contains the las ossim plugin for reading ASPRS LASer(LAS)
data.  Limited support for version 1.2.

# Removing until code changed to use external libraw package.(drb)
%if 0
%package  	    libraw-plugin
Summary:        libraw ossim plugin
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description libraw-plugin
This sub-package contains the libraw ossim plugin for reading data via the
libraw library.
%endif

# libwms does not depend on ossim
%package        wms
Summary:        wms ossim library
Group:          System Environment/Libraries

%description    wms
This sub-package contains the web mapping service (wms) library.

%package  	wms-devel
Summary:        Development files wms
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    wms-devel
This sub-package contains the development files for wms.

%package  	ndf-plugin
Summary:        ndf ossim plugin
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description ndf-plugin
This sub-package contains the ndf ossim plugin for reading National Landsat
Archive Production System (NLAPS) Data Format (NDF).

%package        oms
Summary:        Wrapper library/java bindings for interfacing with ossim.
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    oms
This sub-package contains the oms wrapper library with java bindings for
interfacing with the ossim library from java.

%package        oms-devel
Summary:        Development files for ossim oms wrapper library.
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    oms-devel
This sub-package contains the development files for oms.

%package  	opencv-plugin
Summary:        OSSIM OpenCV plugin, contains registration code.
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description opencv-plugin
This sub-package contains the ossim opencv plugin with various pieces of 
image registration code.

%package 	    ossim-plugin
Summary:        Plugin with various SAR sensor models
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description ossim-plugin
This sub-package contains the ossim plugin which has various SAR sensor models,
readers, and support data parsers.  Most of this code was provided by the ORFEO
Toolbox (OTB) group.

%package 	    planet
Summary:        3D ossim library interface via OpenSceneGraph
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description planet
3D ossim library interface via OpenSceneGraph.

%package 	    planet-devel
Summary:        Development files for ossim planet.
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description planet-devel
This sub-package contains development files for ossim planet.

%package 	    png-plugin
Summary:        PNG ossim plugin
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description png-plugin
This sub-package contains the Portable Network Graphic (png) ossim plugin for
reading/writing png images via the png library. 

%package 	    predator
Summary:        Ossim vedeo library.
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description predator
Ossim vedeo library.

%package 	    predator-devel
Summary:        Development files for ossim planet.
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description predator-devel
This sub-package contains development files for ossim planet.

%package  	    sqlite-plugin
Summary:        OSSIM sqlite plugin, contains GeoPackage reader/writer.
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description sqlite-plugin
This sub-package contains GeoPackage reader/writer.

%package 	    web-plugin
Summary:        web ossim plugin
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description web-plugin
This sub-package contains the web ossim plugin for interfacing with http via
curl library. 


%prep

#---
# Notes for debugging:
# Using the "%setup -q -D -T" is handy debugging.
# -D on setup = Do not delete the directory before unpacking.
# -T on setup = Disable the automatic unpacking of the archives.
#---
# %setup -q -D -T
# %setup -q -D
%setup -q

# Delete bundled libraw
rm -rf ossim_plugins/libraw/LibRaw-0.9.0/


%build

# Exports for ossim build:
export OSSIM_DEV_HOME=%{_builddir}/%{name}-%{version}
export OSSIM_BUILD_DIR=%{_builddir}/%{name}-%{version}/build
export OSSIM_BUILD_TYPE=RelWithDebInfo

mkdir -p build
pushd build
%cmake \
-DBUILD_CSMAPI=OFF \
-DBUILD_OMS=ON \
-DBUILD_OSSIM=ON \
-DBUILD_OSSIM_APPS=ON \
-DBUILD_OSSIM_CURL_APPS=ON \
-DBUILD_CNES_PLUGIN=ON \
-DBUILD_CONTRIB_PLUGIN=ON \
-DBUILD_CSM_PLUGIN=OFF \
-DBUILD_GDAL_PLUGIN=ON \
-DBUILD_GEOPDF_PLUGIN=ON \
-DBUILD_HDF5_PLUGIN=ON \
-DBUILD_KAKADU_PLUGIN=OFF \
-DBUILD_KML_PLUGIN=ON \
-DBUILD_LIBRAW_PLUGIN=ON \
-DBUILD_MRSID_PLUGIN=OFF \
-DBUILD_OSSIMMRSID_PLUGIN=ON \
-DBUILD_NDF_PLUGIN=OFF \
-DBUILD_OPENCV_PLUGIN=OFF \
-DBUILD_OPENJPEG_PLUGIN=OFF \
-DBUILD_PDAL_PLUGIN=OFF \
-DBUILD_PNG_PLUGIN=ON \
-DBUILD_OSSIOPENCV_PLUGIN=ON \
-DBUILD_OSSIMOPENJPEG_PLUGIN=ON \
-DBUILD_REGISTRATION_PLUGIN=OFF \
-DBUILD_SQLITE_PLUGIN=ON \
-DBUILD_WEB_PLUGIN=ON \
-DBUILD_OSSIMGUI=ON \
-DBUILD_OSSIM_MPI_SUPPORT=OFF \
-DBUILD_OSSIMPLANET=ON \
-DBUILD_OSSIMPLANETQT=ON \
-DBUILD_OSSIMPREDATOR=ON \
-DBUILD_OSSIMQT4=OFF \
-DBUILD_OSSIMSQLITE_PLUGIN=ON \
-DBUILD_OSSIM_TEST_APPS=OFF \
-DBUILD_RUNTIME_DIR=bin \
-DBUILD_SHARED_LIBS=ON \
-DBUILD_WMS=ON \
-DCMAKE_BUILD_TYPE=$OSSIM_BUILD_TYPE \
-DCMAKE_MODULE_PATH=$OSSIM_DEV_HOME/ossim_package_support/cmake/CMakeModules \
-DOSSIMPLANET_ENABLE_EPHEMERIS=OFF \
-DCMAKE_PREFIX_PATH=/usr \
../ossim/cmake
make VERBOSE=1 %{?_smp_mflags}
popd

# Exports for java builds:
# export JAVA_HOME=/usr/lib/jvm/java
#export JAVA_HOME=/usr/java/latest
export OSSIM_INSTALL_PREFIX=%{buildroot}/usr
# Build c++ jni bindings and java side of oms module:
pushd ossim-oms/joms
cp local.properties.template local.properties
ant
popd

%install

# Exports for ossim build:
export OSSIM_DEV_HOME=%{_builddir}/%{name}-%{version}
export OSSIM_BUILD_DIR=%{_builddir}/%{name}-%{version}/build
export OSSIM_BUILD_TYPE=RelWithDebInfo
export OSSIM_VERSION=%{RPM_OSSIM_VERSION}

pushd build
make install DESTDIR=%{buildroot}

popd

install -p -m644 -D ossim/support/linux/etc/profile.d/ossim.sh %{buildroot}%{_sysconfdir}/profile.d/ossim.sh
install -p -m644 -D ossim/support/linux/etc/profile.d/ossim.csh %{buildroot}%{_sysconfdir}/profile.d/ossim.csh
install -p -m644 -D ossim/share/ossim/templates/ossim_preferences_template %{buildroot}%{_datadir}/ossim/ossim-preferences-template

# Exports for java builds:
# export JAVA_HOME=/usr/lib/jvm/java
#export JAVA_HOME=/usr/java/latest
export OSSIM_INSTALL_PREFIX=%{buildroot}/usr

# oms "ant" build:
pushd ossim-oms/joms

ant dist
ant install

# mvn-install needed for omar rpm build!
ant mvn-install

# Fix bad perms:
chmod 755 %{buildroot}%{_libdir}/libjoms.so

popd

%post
/sbin/ldconfig

# First time through create the site preferences.
if [ ! -f %{_datadir}/ossim/ossim-site-preferences ]; then
   cp %{_datadir}/ossim/ossim-preferences-template %{_datadir}/ossim/ossim-site-preferences
fi

%postun
/sbin/ldconfig

%post wms -p /sbin/ldconfig
%postun wms -p /sbin/ldconfig

%post oms
/sbin/ldconfig
rm -f %{_javadir}/joms.jar
ln -s %{_javadir}/joms-%{version}.jar %{_javadir}/joms.jar

%postun oms
/sbin/ldconfig
rm -f %{_javadir}/joms.jar

%post planet -p /sbin/ldconfig
%postun planet -p /sbin/ldconfig

%files
%{_bindir}/*
%{_datadir}/ossim/
%doc ossim/LICENSE.txt
%{_libdir}/libossim.so.*
%{_sysconfdir}/profile.d/ossim.sh
%{_sysconfdir}/profile.d/ossim.csh

# Weed out apps:
# %exclude %{_bindir}/ossim-adrg-dump
# %exclude %{_bindir}/ossim-btoa
# %exclude %{_bindir}/ossim-computeSrtmStats
# %exclude %{_bindir}/ossim-correl 
# %exclude %{_bindir}/ossim-create-bitmask
# %exclude %{_bindir}/ossim-dump-ocg
# %exclude %{_bindir}/ossim-image-compare
# %exclude %{_bindir}/ossim-modopt
# %exclude %{_bindir}/ossimplanet
# %exclude %{_bindir}/ossimplanetklv
# %exclude %{_bindir}/ossimplanet-chip
# %exclude %{_bindir}/ossimplanettest
# %exclude %{_bindir}/ossim-rejout
# %exclude %{_bindir}/ossim-rpf 
# %exclude %{_bindir}/ossim-senint
# %exclude %{_bindir}/ossim-space-imaging
# %exclude %{_bindir}/ossim-src2src
# %exclude %{_bindir}/ossim-swapbytes
# %exclude %{_bindir}/ossim-ws-cmp

# These are in the geocell package:
# %exclude %{_bindir}/ossim-geocell
# %exclude %{_bindir}/ossimplanetviewer

%files devel
%{_includedir}/ossim
%{_libdir}/libossim.so
%{_libdir}/pkgconfig/ossim.pc

%files gdal-plugin
%{_libdir}/ossim/plugins/libossimgdal_plugin.so

%files geocell
%{_bindir}/ossim-geocell
%{_libdir}/libossimGui.so*

%files geopdf-plugin
%{_libdir}/ossim/plugins/libossimgeopdf_plugin.so

%if 0
%files hdf-plugin
%{_libdir}/ossim/plugins/libossimhdf_plugin.so
%endif

%if 0 
%files hdf4-plugin
%{_libdir}/ossim/plugins/libossimhdf4_plugin.so
%endif

%if 1
%files hdf5-plugin
%{_libdir}/ossim/plugins/libossimhdf5_plugin.so
%endif

%files kmlsuperoverlay-plugin
%{_libdir}/ossim/plugins/libossimkmlsuperoverlay_plugin.so

%files las-plugin
%{_libdir}/ossim/plugins/libossimlas_plugin.so

# Removing until code changed to use external libraw package.(drb)
%if 0
%files libraw-plugin
%{_libdir}/ossim/plugins/libossimlibraw_plugin.so
%endif

%files wms
%{_libdir}/libwms.so.*

%files wms-devel
%{_includedir}/wms/
%{_libdir}/libwms.so

%files ndf-plugin
%{_libdir}/ossim/plugins/libossimndf_plugin.so

%files oms
%{_javadir}/joms-%{version}.jar
%{_libdir}/libjoms.so
%{_libdir}/liboms.so.*

%files oms-devel
%{_includedir}/oms/
%{_libdir}/liboms.so

%files opencv-plugin
%{_libdir}/ossim/plugins/libossimopencv_plugin.so

%files ossim-plugin
%{_libdir}/ossim/plugins/libossim_plugin.so

%files planet
%{_bindir}/ossimplanet
%{_bindir}/ossimplanetviewer
%{_libdir}/libossimPlanet.so*
%{_libdir}/libossimPlanetQt.so*

%files planet-devel
%{_includedir}/ossimPlanet

%files png-plugin
%{_libdir}/ossim/plugins/libossimpng_plugin.so

%files predator
%{_libdir}/libossimPredator.so*

%files predator-devel
%{_includedir}/ossimPredator

%files sqlite-plugin
%{_libdir}/ossim/plugins/libossimsqlite_plugin.so

%files web-plugin
%{_libdir}/ossim/plugins/libossimweb_plugin.so

%changelog
* Sun Dec 29 2013 Volker Fröhlich <volker27@gmx.at> - yes-1
- Initial package
