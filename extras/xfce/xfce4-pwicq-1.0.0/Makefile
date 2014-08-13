#
# Makefile for the pwICQ's XFCE plugin
#

PROJECT   = icqxfce
PACKET    = xfce4-pwicq
VERSION   = 1.0.0
RELEASE   = 0
SOURCES   = pwicq.c
INCLUDE   = icqdefs.h icqqueue.h icqxpde.h
SUMMARY   = xfce4 Panel Plugin for pwICQ
COPYRIGHT = Perry Werneck
GROUP     = Applications/Internet
PACKAGER  = Perry Werneck
REQUIRES  = xfce4-panel


#---[ This part usually doesn't change ]---------------------------------------#

#
# Rules
#
ifdef DEBUG
   DBGFLAGS = -DDEBUG
else
   DBGFLAGS = -DNDEBUG
endif

GCCFLAGS = -DPROJECT=\"$(PROJECT)\" -DHOSTNAME=\"$(HOSTNAME)\" \
           -DBUILD=\"`date +%G%m%d`\" -DXBUILD=0x`date +%G%m%d` \
		   -DUSER=\"$(USER)\" -Wall $(DBGFLAGS)\
		   $(CFLAGS) \
		   `pkg-config --cflags xfce4-panel-1.0`


%.o : %.c $(INCLUDE)
	@gcc $(GCCFLAGS) -o $@ -c $<
ifdef DEBUG
	@echo $< [DEBUG] Ok.
else
	@echo $< Ok.
endif

%.so : 
	@gcc -shared -Wl,-soname,$(@F) -o $@ $?
	@echo $@ Ok.

%.la : %.so
	@libtool --quiet gcc -o $@ $< -rpath `pkg-config xfce4-panel-1.0 --variable=pluginsdir` -lm
	@echo $@ Ok.

%.tar.gz:
	@rm -fr /tmp/$@
	@mkdir /tmp/$@
	@mkdir /tmp/$@/$(subst .tar.gz,,$@)
	@cp $? /tmp/$@/$(subst .tar.gz,,$@)/
	@tar --create --gzip --remove-files --file=$@ --directory=/tmp/$@ ./$(subst .tar.gz,,$@)
	@rm -fr /tmp/$@
	@echo $@ Ok.
	
%.spec: Makefile
	@rm -f $@
	@echo Summary: $(SUMMARY) >> $@
	@echo Name: $(PACKET) >> $@
	@echo Version: $(VERSION) >> $@
	@echo Copyright: $(COPYRIGHT) >> $@
	@echo Release: $(RELEASE) >> $@
	@echo Group: $(GROUP) >> $@
	@echo Source: %{name}-%{version}.tar.gz >> $@
	@echo Packager: $(PACKAGER) >> $@
	@echo BuildRoot: /var/tmp/%{name}-%{version} >> $@
	@echo Requires: $(REQUIRES) >> $@
	@echo >> $@
	@echo %description >> $@
	@echo >> $@
	@echo %prep >> $@
	@echo  >> $@
	@echo %setup >> $@
	@echo  >> $@
	@echo %build >> $@
	@echo make >> $@
	@echo  >> $@
	@echo %install >> $@
	@echo rm -rf $RPM_BUILD_ROOT >> $@
	@echo -e mkdir -p --mode=755 \\044RPM_BUILD_ROOT`pkg-config xfce4-panel-1.0 --variable=pluginsdir` >> $@
	@echo -e install -m 755 libicqxfce.so \\044RPM_BUILD_ROOT`pkg-config xfce4-panel-1.0 --variable=pluginsdir`/lib$(PROJECT).la >> $@
	@echo -e install -m 755 libicqxfce.so \\044RPM_BUILD_ROOT`pkg-config xfce4-panel-1.0 --variable=pluginsdir`/lib$(PROJECT).so.0 >> $@
	@echo  >> $@
	@echo %clean >> $@
	@echo -e rm -rf \\044RPM_BUILD_ROOT >> $@
	@echo >> $@
	@echo %files >> $@
	@echo -e %defattr\\050-,root,root\\051 >> $@
	@echo `pkg-config xfce4-panel-1.0 --variable=pluginsdir`/lib$(PROJECT).la >> $@
	@echo `pkg-config xfce4-panel-1.0 --variable=pluginsdir`/lib$(PROJECT).so.0 >> $@
	@echo  >> $@
	@echo %config >> $@
	@echo  >> $@
	@echo %pre >> $@
	@echo  >> $@
	@echo %post >> $@
	@echo ln -s `pkg-config xfce4-panel-1.0 --variable=pluginsdir`/lib$(PROJECT).so.0 `pkg-config xfce4-panel-1.0 --variable=pluginsdir`/lib$(PROJECT).so >> $@
	@echo  >> $@
	@echo %preun >> $@
	@echo ln -s `pkg-config xfce4-panel-1.0 --variable=pluginsdir`/lib$(PROJECT).so.0 `pkg-config xfce4-panel-1.0 --variable=pluginsdir`/lib$(PROJECT).so >> $@
	@echo  >> $@
	@echo %postun >> $@
	@echo  >> $@
	@echo $@ Ok!

#
# Build Instructions
#
all:							lib$(PROJECT).la lib$(PROJECT).so

release:						lib$(PROJECT).la lib$(PROJECT).so

tgz:  							$(PACKET)-$(VERSION).tar.gz

spec: $(PACKET).spec
	
floppy:							$(PACKET)-$(VERSION).tar.gz
	@mount /mnt/floppy
	@cp -f $(PACKET)-$(VERSION).tar.gz /mnt/floppy
	@umount /mnt/floppy

$(PACKET)-$(VERSION).tar.gz:	$(SOURCES) $(INCLUDE) Makefile $(PACKET).spec

rpm:							$(PACKET)-$(VERSION).tar.gz $(PACKET).spec
	@cp -f $(PACKET)-$(VERSION).tar.gz `rpm --eval="%{u2p:%{_sourcedir}}"`
	@cp -f $(PACKET).spec `rpm --eval="%{u2p:%{_specdir}}"`
	@rpm -bb `rpm --eval="%{u2p:%{_specdir}}"`/$(PACKET).spec
	@rm -f `rpm --eval="%{u2p:%{_sourcedir}}"`/$(PACKET)-$(VERSION).tar.gz
	@rm -f `rpm --eval="%{u2p:%{_specdir}}"`/$(PACKET).spec
	

lib$(PROJECT).so:				$(SOURCES:.c=.o)

lib$(PROJECT).la:				lib$(PROJECT).so

install:			lib$(PROJECT).la lib$(PROJECT).so
	@echo Installing into `pkg-config xfce4-panel-1.0 --variable=pluginsdir`
	@rm -f /tmp/$(PROJECT).dbg
	@cp -f lib$(PROJECT).la `pkg-config xfce4-panel-1.0 --variable=pluginsdir`
	@cp -f lib$(PROJECT).so `pkg-config xfce4-panel-1.0 --variable=pluginsdir`/lib$(PROJECT).so.0
	@ln -f -s `pkg-config xfce4-panel-1.0 --variable=pluginsdir`/lib$(PROJECT).so.0 \
	          `pkg-config xfce4-panel-1.0 --variable=pluginsdir`/lib$(PROJECT).so
	@killall -USR1 xfce4-panel

clean:
	@rm -f $(SOURCES:.c=.o)
	@rm -f lib$(PROJECT).so
	@rm -f lib$(PROJECT).la
	@rm -f $(PACKET)-$(VERSION).tar.gz
	
