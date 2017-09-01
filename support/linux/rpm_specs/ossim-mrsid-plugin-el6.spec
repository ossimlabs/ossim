Name:           ossim-mrsid-plugin
Version:        %{RPM_OSSIM_VERSION} 
Release:        %{BUILD_RELEASE}%{?dist}
Summary:        OSSIM MrSID Plugin
Group:          System Environment/Libraries
#TODO: Which version?
License:        Mrsid license restrictions.
URL:            http://trac.osgeo.org/ossim/wiki
Source0:        http://download.osgeo.org/ossim/source/%{name}-%{version}.tar.gz

BuildRequires: cmake
BuildRequires: java-devel
BuildRequires: libcurl-devel
BuildRequires: libtiff4-devel
BuildRequires: ossim-devel

%description

OSSIM Mrsid Plugin

%prep
#---
# Notes for debugging: 
# -D on setup = Do not delete the directory before unpacking.
# -T on setup = Disable the automatic unpacking of the archives.
#---
# %setup -q -D -T
%setup -q

%build
OSSIM_DEV_HOME=%{_builddir}/%{name}-%{version}
export OSSIM_BUILD_DIR=%{_builddir}/%{name}-%{version}/build
export OSSIM_BUILD_TYPE=RelWithDebInfo

#build plugin:
mkdir -p build
pushd build

# Note: Linking static libs.
%cmake \
-DCMAKE_BUILD_TYPE=$OSSIM_BUILD_TYPE \
-DCMAKE_MODULE_PATH=$OSSIM_DEV_HOME/CMakeModules \
-DMRSID_DIR=$OSSIM_DEV_HOME/mrsid_code \
../mrsid

make VERBOSE=1 %{?_smp_mflags}

popd

%install

# ossim mrsid plugin:
pushd build
make install DESTDIR=%{buildroot}
popd

# mrsid libraries:
install -p -m755 -D mrsid_code/Lidar_DSDK/lib/liblti_lidar_dsdk.so %{buildroot}%{_libdir}
install -p -m755 -D mrsid_code/Raster_DSDK/lib/libltidsdk.so %{buildroot}%{_libdir}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%{_libdir}/ossim/plugins/libossim_mrsid_plugin.so
%{_libdir}/liblti_lidar_dsdk.so
%{_libdir}/libltidsdk.so

%changelog
* Mon Jan 9 2013 David Burken <dburken@- 1.8.18-1
- Initial package
