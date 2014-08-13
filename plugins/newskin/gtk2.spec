Summary: GTK-2 interface plugin for pwICQ
Name: pwicq-gtk2
Version: 2.0.0
Copyright: Perry Werneck
Release: 0
Group: Applications/Internet
Packager: Perry Werneck
requires: pwicq
BuildRoot: /tmp/icqgtk2.$
conflicts: pwicqgnome pwicq-gtk1

%description
GTK-1 Interface Plugin for pwICQ

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/share/pwicq/plugins

install -m 666 /usr/share/pwicq/plugins/icqgtk2.so $RPM_BUILD_ROOT/usr/share/pwicq/plugins

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/share/pwicq/plugins/icqgtk2.so

%config

%pre
/usr/share/pwicq/pwicq-loader --stop --nodaemon

%preun
/usr/share/pwicq/pwicq-loader --stop --nodaemon

%post

%postun

