Summary: LIBOSD Plugin for pwICQ
Name: ICQOSDPlugin
Version: 1.0.0
Copyright: Perry Werneck
Release: 0
Group: Applications/Internet
Packager: Perry Werneck
requires: pwicq xosd
BuildRoot: /tmp/icqosd.$

%description
pwICQ Plugin for OSD Status messages

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/share/pwicq/plugins

install -m 666 /usr/share/pwicq/plugins/icqosd.so  $RPM_BUILD_ROOT/usr/share/pwicq/plugins

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

/usr/share/pwicq/plugins/icqosd.so

%config

%pre
/usr/share/pwicq/pwicq-loader --stop --nodaemon

%preun
/usr/share/pwicq/pwicq-loader --stop --nodaemon

%post

%postun

