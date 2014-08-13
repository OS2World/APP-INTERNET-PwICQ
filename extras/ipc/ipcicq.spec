Summary: pwICQ - Rexx/IPC Library
Name: icqipc
Version: 2.0.0
Copyright: Perry Werneck
Release: 0
Group: Applications/System
Packager: Perry Werneck
requires: pwicq
BuildRoot: /tmp/icqipc.$

%description
pwICQ IPC interface, including REXX extensions

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/share/pwicq
mkdir -p $RPM_BUILD_ROOT/usr/share/pwicq/scripts

install -m 555 /usr/share/pwicq/icqipc.so $RPM_BUILD_ROOT/usr/share/pwicq
install -m 555 /mnt/os2data/public/pwicq/linux/bin/scripts/ishare $RPM_BUILD_ROOT/usr/share/pwicq/scripts

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

/usr/share/pwicq/icqipc.so
/usr/share/pwicq/scripts/ishare

%config

%pre

%post
ln -s /usr/share/pwicq/icqipc.so /usr/lib/libicqipc.so

%postun
rm -f /usr/lib/libicqipc.so
rm -f /usr/share/share/pwicq/icqipc.so


