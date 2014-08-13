Summary: KDE3 Panel applet for pwICQ Version2
Name: pwicqKDE3Applet
Version: 1.0.0
Copyright: Perry Werneck
Release: 1
Group: Applications/Internet
Packager: Perry Werneck
requires: pwicq
BuildRoot: /tmp/pwicq/KDE3Applet.$

%description
KDE3 Panel applet for controling some pwICQ functions

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p --mode=755 $RPM_BUILD_ROOT/usr/lib/kde3
mkdir -p --mode=755 $RPM_BUILD_ROOT/usr/share/apps/kicker/applets
mkdir -p --mode=755 $RPM_BUILD_ROOT/usr/share/pwicq

install -m 755 /usr/share/pwicq/libpwicqkde_applet.so.0.0.0 $RPM_BUILD_ROOT/usr/share/pwicq
install -m 755 /usr/share/pwicq/libpwicqkde_applet.la $RPM_BUILD_ROOT/usr/share/pwicq
install -m 644 /usr/share/apps/kicker/applets/pwicqapplet.desktop $RPM_BUILD_ROOT/usr/share/apps/kicker/applets

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

/usr/share/pwicq/libpwicqkde_applet.so.0.0.0
/usr/share/pwicq/libpwicqkde_applet.la
/usr/share/apps/kicker/applets/pwicqapplet.desktop

%config

%pre

%post
ln -s /usr/share/pwicq/libpwicqkde_applet.so.0.0.0 /usr/lib/kde3/libpwicqkde_applet.so.0.0.0
ln -s /usr/share/pwicq/libpwicqkde_applet.la /usr/lib/kde3/libpwicqkde_applet.la
ln -s /usr/lib/kde3/libpwicqkde_applet.so.0.0.0 /usr/lib/kde3/libpwicqkde_applet.so.0
ln -s /usr/lib/kde3/libpwicqkde_applet.so.0.0.0 /usr/lib/kde3/libpwicqkde_applet.so

%preun
rm -f /usr/lib/kde3/libpwicqkde_applet.so
rm -f /usr/lib/kde3/libpwicqkde_applet.so.0
rm -f /usr/lib/kde3/libpwicqkde_applet.la
rm -f /usr/lib/kde3/libpwicqkde_applet.so.0.0.0


