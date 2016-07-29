Name: mod_dav_castor
Version: 0.1
Release: 1
License: Unreleased
Group: Applications/Internet
Requires: castor-lib httpd
BuildRequires: castor-devel httpd-devel
Summary: Apache httpd module providing a WebDAV interface to CASTOR.
Source: mod_dav_castor-%{version}.tar.gz
Buildroot: %{_tmppath}/%{name}-root

%description
Apache httpd module providing a WebDAV interface to the CASTOR storage system.

%prep
%setup

%build
cd src
make

%install
cd src
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/lib64/httpd/modules/mod_dav_castor.so
