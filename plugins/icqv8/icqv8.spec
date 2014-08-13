Summary: ICQV8 Protocol manager plugin for pwICQ
Name: ICQV8ProtocolManager
Version: 2.0.0
Copyright: Perry Werneck
Release: 0
Group: Applications/Internet
Packager: Perry Werneck
requires: pwicq
conflicts: ICQV7ProtocolManager
BuildRoot: /tmp/icqv8.$

%description
ICQ Version 8 protocol manager for pwICQ

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/share/pwicq/plugins

install -m 666 /usr/share/pwicq/plugins/icqv8.so  $RPM_BUILD_ROOT/usr/share/pwicq/plugins
install -m 666 /usr/share/pwicq/plugins/icqv8p.so $RPM_BUILD_ROOT/usr/share/pwicq/plugins

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

/usr/share/pwicq/plugins/icqv8.so
/usr/share/pwicq/plugins/icqv8p.so

%config

%pre
/usr/share/pwicq/pwicq-loader --stop --nodaemon

%preun
/usr/share/pwicq/pwicq-loader --stop --nodaemon

%post

%postun

