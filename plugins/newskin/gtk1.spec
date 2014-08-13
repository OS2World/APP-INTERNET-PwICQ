Summary: GTK-1 interface plugin for pwICQ
Name: pwicq-gtk1
Version: 2.0.0
Copyright: Perry Werneck
Release: 0
Group: Applications/Internet
Packager: Perry Werneck
requires: pwicq
BuildRoot: /tmp/icqgtk1.$
conflicts: pwicqgnome pwicq-gtk2

%description
GTK-1 Interface Plugin for pwICQ

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/share/pwicq/plugins

install -m 666 /mnt/os2data/public/pwicq/linux/experimental/icqgtk1.so $RPM_BUILD_ROOT/usr/share/pwicq/plugins

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

/usr/share/pwicq/plugins/icqgtk1.so

%config

%pre
/usr/share/pwicq/pwicq-loader --stop --nodaemon

%preun
/usr/share/pwicq/pwicq-loader --stop --nodaemon

%post

%postun

