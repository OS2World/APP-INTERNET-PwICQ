Summary: pwICQ - ICQ Clone (Core)
Name: pwicq-core
Version: 2.0.0
Copyright: Perry Werneck
Release: 1
Group: Applications/Internet
Packager: Perry Werneck
BuildRoot: /tmp/pwicq.$
Provides: pwicq icqnetw.so

%description
pwICQ - Perry Werneck ICQ clone (Core modules)

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p --mode=755 $RPM_BUILD_ROOT/usr/share/pwicq
mkdir -p --mode=755 $RPM_BUILD_ROOT/usr/share/pwicq/images
mkdir -p --mode=755 $RPM_BUILD_ROOT/usr/share/pwicq/plugins
mkdir -p --mode=755 $RPM_BUILD_ROOT/usr/share/pwicq/sounds

install -m 755 /usr/share/pwicq/pwicq.sh            $RPM_BUILD_ROOT/usr/share/pwicq

install -m 755 /usr/share/pwicq/icqnetw.so          $RPM_BUILD_ROOT/usr/share/pwicq
install -m 755 /usr/share/pwicq/pwicq               $RPM_BUILD_ROOT/usr/share/pwicq
install -m 755 /usr/share/pwicq/icqkrnl.so          $RPM_BUILD_ROOT/usr/share/pwicq

install -m 755 /usr/share/pwicq/plugins/icqconf.so  $RPM_BUILD_ROOT/usr/share/pwicq/plugins
install -m 755 /usr/share/pwicq/plugins/icqmsg.so   $RPM_BUILD_ROOT/usr/share/pwicq/plugins
install -m 755 /usr/share/pwicq/plugins/icqulog.so  $RPM_BUILD_ROOT/usr/share/pwicq/plugins
install -m 755 /usr/share/pwicq/plugins/icqusrdb.so $RPM_BUILD_ROOT/usr/share/pwicq/plugins

install -m 444 /usr/share/pwicq/images/icons.gif    $RPM_BUILD_ROOT/usr/share/pwicq/images

install -m 444 /usr/share/pwicq/pwicq.png           $RPM_BUILD_ROOT/usr/share/pwicq

install -m 444 /usr/share/pwicq/ignore.dat          $RPM_BUILD_ROOT/usr/share/pwicq

install -m 444 /usr/share/pwicq/blacklist.dat       $RPM_BUILD_ROOT/usr/share/pwicq
install -m 444 /usr/share/pwicq/events.dat          $RPM_BUILD_ROOT/usr/share/pwicq
install -m 444 /usr/share/pwicq/badword.dat         $RPM_BUILD_ROOT/usr/share/pwicq

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

/usr/share/pwicq/pwicq
/usr/share/pwicq/pwicq.sh
/usr/share/pwicq/icqnetw.so
/usr/share/pwicq/icqkrnl.so

/usr/share/pwicq/plugins/icqconf.so
/usr/share/pwicq/plugins/icqmsg.so
/usr/share/pwicq/plugins/icqulog.so
/usr/share/pwicq/plugins/icqusrdb.so

/usr/share/pwicq/images/icons.gif

/usr/share/pwicq/events.dat

/usr/share/pwicq/pwicq.png

/usr/share/pwicq/badword.dat
/usr/share/pwicq/blacklist.dat
/usr/share/pwicq/ignore.dat

%config

%pre

%post
ln -s /usr/share/pwicq/pwicq.sh /usr/bin/pwicq
ln -s /usr/share/pwicq/pwicq    /usr/share/pwicq/pwicq-loader

%preun
/usr/share/pwicq/pwicq --stop --nodaemon

%postun
rm -f /usr/bin/pwicq
rm -f /usr/lib/icqnetw.so
rm -fr /usr/share/pwicq


