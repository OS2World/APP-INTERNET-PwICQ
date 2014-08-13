Summary: pwICQ - Command line interface
Name: icqcmd
Version: 1.0.0
Copyright: Perry Werneck
Release: 0
Group: Applications/Internet
Packager: Perry Werneck
BuildRoot: /tmp/icqcmd.$

%description
pwICQ - Command line interface

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin

install -m 755 /usr/bin/icqcmd    $RPM_BUILD_ROOT/usr/bin/icqcmd

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

/usr/bin/icqcmd

%config

%pre

%post

%preun

%postun


