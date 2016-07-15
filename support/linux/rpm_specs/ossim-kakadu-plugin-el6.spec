#---
# File: ossim-kakadu-el7.spec
#
# Spec file for building ossim rpms with rpmbuild.
#
# NOTE: This files differs from ossim-el6.spec on in el6 need libtiff4 to pick
# up bigtiff support.
#
# Example usage:
# rpmbuild -ba --define 'RPM_OSSIM_VERSION 1.9.0' --define 'BUILD_RELEASE 1' ossim-kakadu-el7.spec
#---


Name:           ossim-kakadu-plugin
Version:        %{RPM_OSSIM_VERSION} 
Release:        %{BUILD_RELEASE}%{?dist}
Summary:        OSSIM Kakadu Plugin
Group:          System Environment/Libraries
#TODO: Which version?
License:        Kakadu license restrictions.
URL:            http://trac.osgeo.org/ossim/wiki
Source0:        http://download.osgeo.org/ossim/source/%{name}-%{version}.tar.gz

BuildRequires: cmake
BuildRequires: java-devel
BuildRequires: libcurl-devel
BuildRequires: libtiff4-devel
BuildRequires: ossim-devel

# Kakadu version used:
# %define KAKADU_VERSION v7_7-01123C
%define KAKADU_VERSION kakadu_src

%description

OSSIM Kakadu Plugin

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
#export JAVA_HOME=/usr/java/latest
export OSSIM_BUILD_DIR=%{_builddir}/%{name}-%{version}/build
export OSSIM_BUILD_TYPE=RelWithDebInfo

# Build kakadu:
pushd %{KAKADU_VERSION}/make
make -f Makefile-Linux-x86-64-gcc
popd

#build plugin:
mkdir -p build
pushd build

# Note: Linking static libs.
%cmake \
-DCMAKE_BUILD_TYPE=${OSSIM_BUILD_TYPE} \
-DCMAKE_MODULE_PATH=$OSSIM_DEV_HOME/CMakeModules \
-DKAKADU_ROOT_SRC=$OSSIM_DEV_HOME/%{KAKADU_VERSION} \
-DKAKADU_AUX_LIBRARY=$OSSIM_DEV_HOME/%{KAKADU_VERSION}/lib/Linux-x86-64-gcc/libkdu_aux.a \
-DKAKADU_LIBRARY=$OSSIM_DEV_HOME/%{KAKADU_VERSION}/lib/Linux-x86-64-gcc/libkdu.a  \
../kakadu

make VERBOSE=1 %{?_smp_mflags}

popd

%install
pushd build
make install DESTDIR=%{buildroot}
popd

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%{_libdir}/ossim/plugins/libossim_kakadu_plugin.so

%changelog
* Mon Jan 6 2013 David Burken <dburken@radiantblue.com> 1.8.18-1
- Initial package
