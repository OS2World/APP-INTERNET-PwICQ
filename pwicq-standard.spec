Summary: pwICQ - ICQ Clone (Standard packet)
Name: pwicq-standard
Version: 2.0.0
Copyright: Perry Werneck
Release: 1
Group: Applications/Internet
Packager: Perry Werneck
BuildRoot: /tmp/pwicq-standard.$
Provides: pwicq icqcmd icqnetw.so icqgtk ICQV8ProtocolManager pwICQRexx
conflicts: ICQV7ProtocolManager

%description
pwICQ - Perry Werneck's ICQ clone (Standard packet)

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p --mode=755 $RPM_BUILD_ROOT/usr/bin
mkdir -p --mode=755 $RPM_BUILD_ROOT/usr/share/pwicq
mkdir -p --mode=755 $RPM_BUILD_ROOT/usr/share/pwicq/images
mkdir -p --mode=755 $RPM_BUILD_ROOT/usr/share/pwicq/plugins
mkdir -p --mode=755 $RPM_BUILD_ROOT/usr/share/pwicq/eventhelpers
mkdir -p --mode=755 $RPM_BUILD_ROOT/usr/share/pwicq/sounds

install -m 755 /usr/share/pwicq/pwicq               $RPM_BUILD_ROOT/usr/share/pwicq
install -m 755 /usr/share/pwicq/pwicq.sh            $RPM_BUILD_ROOT/usr/share/pwicq

install -m 755 /usr/bin/icqcmd                      $RPM_BUILD_ROOT/usr/bin

install -m 755 /usr/share/pwicq/icqnetw.so          $RPM_BUILD_ROOT/usr/share/pwicq
install -m 755 /usr/share/pwicq/icqkrnl.so          $RPM_BUILD_ROOT/usr/share/pwicq

install -m 755 /usr/share/pwicq/plugins/icqconf.so  $RPM_BUILD_ROOT/usr/share/pwicq/plugins
install -m 755 /usr/share/pwicq/plugins/icqmsg.so   $RPM_BUILD_ROOT/usr/share/pwicq/plugins
install -m 755 /usr/share/pwicq/plugins/icqulog.so  $RPM_BUILD_ROOT/usr/share/pwicq/plugins
install -m 755 /usr/share/pwicq/plugins/icqusrdb.so $RPM_BUILD_ROOT/usr/share/pwicq/plugins

install -m 644 /usr/share/pwicq/images/icons.gif    $RPM_BUILD_ROOT/usr/share/pwicq/images

install -m 644 /usr/share/pwicq/pwicq.png           $RPM_BUILD_ROOT/usr/share/pwicq

install -m 644 /usr/share/pwicq/ignore.dat          $RPM_BUILD_ROOT/usr/share/pwicq

install -m 644 /usr/share/pwicq/blacklist.dat       $RPM_BUILD_ROOT/usr/share/pwicq
install -m 644 /usr/share/pwicq/events.dat          $RPM_BUILD_ROOT/usr/share/pwicq
install -m 644 /usr/share/pwicq/badword.dat         $RPM_BUILD_ROOT/usr/share/pwicq

install -m 755 /usr/share/pwicq/plugins/icqgtk2.so  $RPM_BUILD_ROOT/usr/share/pwicq/plugins

install -m 755 /usr/share/pwicq/plugins/icqsound.so $RPM_BUILD_ROOT/usr/share/pwicq/plugins

install -m 755 /usr/share/pwicq/plugins/icqv8.so    $RPM_BUILD_ROOT/usr/share/pwicq/plugins
install -m 755 /usr/share/pwicq/plugins/icqv8p.so   $RPM_BUILD_ROOT/usr/share/pwicq/plugins

install -m 755 /usr/share/pwicq/plugins/icqrexx.so  $RPM_BUILD_ROOT/usr/share/pwicq/plugins

install -m 755 /usr/share/pwicq/plugins/isicq.so    $RPM_BUILD_ROOT/usr/share/pwicq/plugins

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

/usr/bin/icqcmd
/usr/share/pwicq/pwicq
/usr/share/pwicq/pwicq.sh

/usr/share/pwicq/icqnetw.so
/usr/share/pwicq/icqkrnl.so

/usr/share/pwicq/plugins/icqconf.so
/usr/share/pwicq/plugins/icqmsg.so
/usr/share/pwicq/plugins/icqulog.so
/usr/share/pwicq/plugins/icqusrdb.so

/usr/share/pwicq/plugins/icqgtk2.so
/usr/share/pwicq/plugins/icqsound.so

/usr/share/pwicq/plugins/icqv8.so
/usr/share/pwicq/plugins/icqv8p.so

/usr/share/pwicq/plugins/icqrexx.so

/usr/share/pwicq/plugins/isicq.so

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
if [ -d /usr/share/icons ]; then ln -s /usr/share/pwicq/pwicq.png /usr/share/icons ; fi

%preun
/usr/share/pwicq/pwicq --stop --nodaemon

%postun
rm -f  /usr/bin/pwicq
rm -fr /usr/share/pwicq


