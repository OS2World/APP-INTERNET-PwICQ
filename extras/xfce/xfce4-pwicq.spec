Summary: xfce4 Panel Plugin for pwICQ
Name: xfce4-pwicq
Version: 1.0.0
Copyright: Perry Werneck
Release: 0
Group: Applications/Internet
Source: %{name}-%{version}.tar.gz
Packager: Perry Werneck
BuildRoot: /var/tmp/%{name}-%{version}
Requires: xfce4-panel

%description

%prep

%setup

%build
make

%install
rm -rf PM_BUILD_ROOT
mkdir -p --mode=755 $RPM_BUILD_ROOT/usr/local/lib/xfce4/panel-plugins
install -m 755 libicqxfce.so $RPM_BUILD_ROOT/usr/local/lib/xfce4/panel-plugins/libicqxfce.la
install -m 755 libicqxfce.so $RPM_BUILD_ROOT/usr/local/lib/xfce4/panel-plugins/libicqxfce.so.0

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/local/lib/xfce4/panel-plugins/libicqxfce.la
/usr/local/lib/xfce4/panel-plugins/libicqxfce.so.0

%config

%pre

%post
ln -s /usr/local/lib/xfce4/panel-plugins/libicqxfce.so.0 /usr/local/lib/xfce4/panel-plugins/libicqxfce.so
killall -USR1 xfce4-panel

%preun
ln -s /usr/local/lib/xfce4/panel-plugins/libicqxfce.so.0 /usr/local/lib/xfce4/panel-plugins/libicqxfce.so

%postun

