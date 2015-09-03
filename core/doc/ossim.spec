# $Id: ossim.spec 16 2001-02-08 11:04:54Z dmetz $
# $crtd:  by  Derald Metzger  on  000226 $
# $cmnt:  spec file for ossim $

# Preamble
Summary: Open Source Software Image Map
Name: ossim
Version: 0.999.7
Release: 1
Copyright: GPL LGPL
Group: Applications/Image
Source: ftp://ftp.remotesensing.org/pub/ossim/%{name}-%{version}.tgz
URL: http://www.remotesensing.org/ossim/
Distribution: na
Vendor: imagelinks.com
Packager: Derald Metzger <dmetz@remotesensing.org>
BuildRoot: %{_tmppath}/%{name}-%{version}
#Provides: 
#Requires: 

%description
 ######--- This is alpha stage development code. ---######
 OSSIM (Open Source Software Image Map) project. 
Pronounced "awesome", the OSSIM project leverages existing open source 
algorithms, tools, and packages to construct an integrated library for 
remote sensing, image-processing, and Geographical Information Sciences (GIS) 
analysis.

%prep
%setup
rm -r `find . -name CVS -print`

%build
./configure \
    --prefix=%{_prefix} \
    --with-grass=no \
    --without-python
make

%install
make \
  version=%{version} \
  prefix=$RPM_BUILD_ROOT%{_prefix} \
  sysconfdir=$RPM_BUILD_ROOT%{_sysconfdir} \
  install

%changelog
* Wed Feb 08 2001  Derald Metzger <dmetz@remotesensing.org>
-UPDATE pkg to 0.999.7-1
  The ossim dir tree has been extensively reconfigured.
* Thu Dec 28 2000  Derald Metzger <dmetz@remotesensing.org>
-UPDATE pkg to 0.999.3-1
  The 0.999 is meant to imply preliminary code (alpha or beta).
  The .3-1 is the 3rd preliminary version and first rpm release of it. 
- Remove $BR build root stuff from Makefile. Use make var defs on cmdline.
- Macroized and updated the files manifest
- Added config dir etc/ossim
- Added doc dir
- Removed lib/ossim - linking statically
* Thu Aug 31 2000  Derald Metzger <dmetz@remotesensing.org>
-pkg 0.2.0a-2  Rebuild with wxGTK-2.2.1-0.
* Sat Aug 26 2000  Derald Metzger <dmetz@remotesensing.org>
-pkg 0.2.0a-1  Snapshot of current alpha code
* Thu May 23 2000  Derald Metzger <dmetz@remotesensing.org>
-PACKGED 0.0.0-1
- rpm'd ossim. Started version at 0.0.0

%files
%defattr(-,root,root)
%{_bindir}/%{name}
%doc %{_docdir}/%{name}-%{version}/
%config %{_sysconfdir}/%{name}






