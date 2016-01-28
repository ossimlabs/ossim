#---
# File: ossim-el7.spec
#
# Spec file for building ossim rpms with rpmbuild.
#
# NOTE: This files differs from ossim-el6.spec on in el6 need libtiff4 to pick
# up bigtiff support.
#
# Example usage:
# rpmbuild -ba --define 'RPM_OSSIM_VERSION 1.9.0' --define 'BUILD_RELEASE 1' ossim-el7.spec
#
# Caveats: 
# 1) Builder/user needs "groovy" in their search path.
# 2) Use "archive.sh" script in ossim/scripts/archive.sh to generate the source
#    tar ball, e.g. ossim-1.9.0.tar.gz, from appropriate git branch.
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

%package 	devel
Summary:        Develelopment files for ossim
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
Development files for ossim.

%package 	libs
Summary:        Develelopment files for ossim
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description libs
Libraries for ossim.

%package 	geocell
Summary:        Desktop electronic light table
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description geocell
Desktop electronic light table for geospatial image processing. Has 2D, 2 1/2D
and 3D viewer with image chain editing capabilities.

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

%package 	planet
Summary:        3D ossim library interface via OpenSceneGraph
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description planet
3D ossim library interface via OpenSceneGraph.

%package 	planet-devel
Summary:        Development files for ossim planet.
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description planet-devel
This sub-package contains development files for ossim planet.

%package        test-apps
Summary:        Ossim test apps.
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    test-apps
A suite of ossim test apps.

%package 	video
Summary:        Ossim vedeo library.
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    video
Ossim vedeo library.

%package 	video-devel
Summary:        Development files for ossim planet.
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    video-devel
This sub-package contains development files for ossim planet.

# libwms does not depend on ossim
%package        wms
Summary:        wms ossim library
Group:          System Environment/Libraries

%description    wms
This sub-package contains the web mapping service (wms) library.

%package 	wms-devel
Summary:        Development files libwms
Group:          System Environment/Libraries
Requires:       libwms%{?_isa} = %{version}-%{release}

%description    wms-devel
This sub-package contains the development files for libwms.

#---
# ossim plugins:
#---
%package 	cnes-plugin
Summary:        Plugin with various sensor models
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description cnes-plugin
This sub-package contains the ossim plugin which has various SAR sensor models,
readers, and support data parsers.  Most of this code was provided by the ORFEO
Toolbox (OTB) group / Centre national d'études spatiales.

%package 	gdal-plugin
Summary:        GDAL ossim plugin
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description gdal-plugin
This sub-package contains the Geospatial Data Abstraction Library(gdal) ossim
plugin for reading/writing images supported by the gdal library.

%package 	geopdf-plugin
Summary:        geopdf ossim plugin
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description geopdf-plugin
This sub-package contains the geopdf ossim plugin for reading geopdf files via
the podofo library.

%package 	hdf5-plugin
Summary:        HDF5 ossim plugin
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description hdf5-plugin
This sub-package contains the Hierarchical Data Format(hdf) ossim plugin for
reading hdf5 images via the hdf5 libraries

%package  	kml-plugin
Summary:        kml ossim plugin
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    kml-plugin
This sub-package contains the kmlsuperoverlay ossim plugin for reading/writing
kml super overlays.

%package  	opencv-plugin
Summary:        OSSIM OpenCV plugin, contains registration code.
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    opencv-plugin
This sub-package contains the ossim opencv plugin with various pieces of 
image registration code.

%package  	openjpeg-plugin
Summary:        OpenJPEG ossim plugin
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    openjpeg-plugin
This sub-package contains the OpenJPEG ossim plugin for
reading/writing J2K compressed images via the OpenJPEG library.

%package 	png-plugin
Summary:        PNG ossim plugin
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description png-plugin
This sub-package contains the Portable Network Graphic(png) ossim plugin for
reading/writing png images via the png library.

%package  	sqlite-plugin
Summary:        OSSIM sqlite plugin, contains GeoPackage reader/writer.
Group:          System Environment/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description sqlite-plugin
This sub-package contains GeoPackage reader/writer.

%package 	web-plugin
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
%setup -q -D -T
# %setup -q -D
# %setup -q

# Delete bundled libraw
rm -rf ossim_plugins/libraw/LibRaw-0.9.0/


%build

# Exports for ossim abuild:
export OSSIM_DEV_HOME=%{_builddir}/%{name}-%{version}
export OSSIM_BUILD_DIR=%{_builddir}/%{name}-%{version}/build
export OSSIM_BUILD_TYPE=RelWithDebInfo
export OSSIM_INSTALL_PREFIX=/usr
export OSSIM_VERSION=%{RPM_OSSIM_VERSION}

mkdir -p build
pushd build
%cmake \
-DBUILD_CSMAPI=OFF \
-DBUILD_OMS=ON \
-DBUILD_OSSIM=ON \
-DBUILD_OSSIM_APPS=ON \
-DBUILD_OSSIM_CURL_APPS=ON \
-DBUILD_OSSIM_GUI=ON \
-DBUILD_OSSIM_TESTS=ON \
-DBUILD_OSSIM_MPI_SUPPORT=OFF \
-DBUILD_OSSIM_PLANET=ON \
-DBUILD_OSSIM_PLANET_QT=OFF \
-DBUILD_OSSIM_VIDEO=ON \
\
-DBUILD_CNES_PLUGIN=ON \
-DBUILD_CONTRIB_PLUGIN=OFF \
-DBUILD_CSM_PLUGIN=OFF \
-DBUILD_GDAL_PLUGIN=ON \
-DBUILD_GEOPDF_PLUGIN=ON \
-DBUILD_HDF5_PLUGIN=ON \
-DBUILD_KAKADU_PLUGIN=OFF \
-DBUILD_KML_PLUGIN=ON \
-DBUILD_LIBRAW_PLUGIN=OFF \
-DBUILD_MRSID_PLUGIN=OFF \
-DBUILD_OSSIMMRSID_PLUGIN=ON \
-DBUILD_NDF_PLUGIN=OFF \
-DBUILD_OPENCV_PLUGIN=ON \
-DBUILD_OPENJPEG_PLUGIN=ON \
-DBUILD_PDAL_PLUGIN=OFF \
-DBUILD_PNG_PLUGIN=ON \
-DBUILD_REGISTRATION_PLUGIN=OFF \
-DBUILD_SQLITE_PLUGIN=ON \
-DBUILD_WEB_PLUGIN=ON \
\
-DBUILD_RUNTIME_DIR=bin \
-DBUILD_SHARED_LIBS=ON \
-DBUILD_WMS=ON \
\
-DOSSIMPLANET_ENABLE_EPHEMERIS=OFF \
\
-DCMAKE_BUILD_TYPE=$OSSIM_BUILD_TYPE \
-DCMAKE_MODULE_PATH=$OSSIM_DEV_HOME/ossim_package_support/cmake/CMakeModules \
-DCMAKE_PREFIX_PATH=/usr \
../ossim/cmake
make VERBOSE=1 %{?_smp_mflags}
popd


# Exports for java builds:
export JAVA_HOME=/usr/lib/jvm/java
#export JAVA_HOME=/usr/java/latest

# Build c++ jni bindings and java side of oms module:
pushd ossim-oms/joms
cp local.properties.template local.properties
ant
popd

%install

# Exports for ossim install:
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
export JAVA_HOME=/usr/lib/jvm/java
#export JAVA_HOME=/usr/java/latest
export OSSIM_INSTALL_PREFIX=%{buildroot}/usr

# oms "ant" build:
pushd ossim-oms/joms
ant dist
ant install
# ant mvn-install
# Fix bad perms:
chmod 755 %{buildroot}%{_libdir}/libjoms.so
popd


%post
/sbin/ldconfig

# First time through create the site preferences.
if [ ! -f %{_datadir}/ossim/ossim-site-preferences ]; then
   cp %{_datadir}/ossim/ossim-preferences-template %{_datadir}/ossim/ossim-site-preferences
fi

%post oms
/sbin/ldconfig
rm -f %{_javadir}/joms.jar
ln -s %{_javadir}/joms-%{version}.jar %{_javadir}/joms.jar

%post planet
/sbin/ldconfig

%post wms
/sbin/ldconfig

%postun
/sbin/ldconfig

%postun oms
/sbin/ldconfig
rm -f %{_javadir}/joms.jar

%postun planet
/sbin/ldconfig

%postun wms
/sbin/ldconfig


%files
%{_bindir}/*

# Weed out apps:
%exclude %{_bindir}/ossim-*-test

%exclude %{_bindir}/ossim-adrg-dump
%exclude %{_bindir}/ossim-btoa
%exclude %{_bindir}/ossim-computeSrtmStats
%exclude %{_bindir}/ossim-correl 
%exclude %{_bindir}/ossim-create-bitmask
%exclude %{_bindir}/ossim-dump-ocg
%exclude %{_bindir}/ossim-image-compare
%exclude %{_bindir}/ossim-modopt
%exclude %{_bindir}/ossimplanetklv
%exclude %{_bindir}/ossimplanet-chip
%exclude %{_bindir}/ossimplanettest
%exclude %{_bindir}/ossim-rejout
%exclude %{_bindir}/ossim-rpf 
%exclude %{_bindir}/ossim-senint
%exclude %{_bindir}/ossim-space-imaging
%exclude %{_bindir}/ossim-src2src
%exclude %{_bindir}/ossim-swapbytes
%exclude %{_bindir}/ossim-ws-cmp

# These are in the geocell package:
%exclude %{_bindir}/ossim-geocell
%exclude %{_bindir}/ossimplanetviewer

%files devel
%{_includedir}/ossim

%files libs
%{_datadir}/ossim/
%doc ossim/LICENSE.txt
%{_libdir}/libossim.so*
%{_libdir}/pkgconfig/ossim.pc
%{_sysconfdir}/profile.d/ossim.sh
%{_sysconfdir}/profile.d/ossim.csh

%files geocell
%{_bindir}/ossim-geocell
%{_libdir}/libossimGui.so*

%files oms
%{_javadir}/joms-%{version}.jar
%{_libdir}/libjoms.so
%{_libdir}/liboms.so*

%files oms-devel
%{_includedir}/oms/

%files planet
# %{_bindir}/ossimplanet
%{_bindir}/ossimplanetviewer
%{_libdir}/libossim-planet.so*
# %{_libdir}/libossimPlanetQt.so*

%files planet-devel
%{_includedir}/ossimPlanet

%files test-apps
%{_bindir}/ossim-*-test

%files video
%{_libdir}/libossim-video.so*

%files video-devel
%{_includedir}/ossimPredator

%files wms
%{_includedir}/wms/
%{_libdir}/libossim-wms.so*

#---
# ossim plugins
#---
%files cnes-plugin
%{_libdir}/ossim/plugins/libossim_cnes_plugin.so

%files gdal-plugin
%{_libdir}/ossim/plugins/libossim_gdal_plugin.so

%files geopdf-plugin
%{_libdir}/ossim/plugins/libossim_geopdf_plugin.so

%files hdf5-plugin
%{_libdir}/ossim/plugins/libossim_hdf5_plugin.so

%files kml-plugin
%{_libdir}/ossim/plugins/libossim_kml_plugin.so

%files opencv-plugin
%{_libdir}/ossim/plugins/libossim_opencv_plugin.so

%files openjpeg-plugin
%{_libdir}/ossim/plugins/libossim_openjpeg_plugin.so

%files png-plugin
%{_libdir}/ossim/plugins/libossim_png_plugin.so

%files sqlite-plugin
%{_libdir}/ossim/plugins/libossim_sqlite_plugin.so

%files web-plugin
%{_libdir}/ossim/plugins/libossim_web_plugin.so



%changelog
* Sun Dec 29 2013 Volker Fröhlich <volker27@gmx.at> - yes-1
- Initial package
