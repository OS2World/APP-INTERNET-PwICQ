Summary: Rexx/Script manager plugin for pwICQ
Name: pwICQRexx
Version: 2.0.0
Copyright: Perry Werneck
Release: 0
Group: Applications/Internet
Packager: Perry Werneck
requires: pwicq
BuildRoot: /tmp/icqvrexx.$

%description
Rexx/Script manager plugin for pwICQ

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/share/pwicq/plugins
mkdir -p $RPM_BUILD_ROOT/usr/share/pwicq/eventhelpers

install -m 555 /usr/share/pwicq/plugins/icqrexx.so $RPM_BUILD_ROOT/usr/share/pwicq/plugins
install -m 555 /usr/share/pwicq/eventhelpers/icqhostupdate $RPM_BUILD_ROOT/usr/share/pwicq/eventhelpers

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

/usr/share/pwicq/plugins/icqrexx.so
/usr/share/pwicq/eventhelpers/icqhostupdate

%config

%pre
/usr/share/pwicq/pwicq-loader --stop --nodaemon

%preun
/usr/share/pwicq/pwicq-loader --stop --nodaemon

%post

%postun

