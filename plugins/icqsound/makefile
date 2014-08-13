
#---[ Dados do projeto ]---------------------------------------------------------------#

PROJECT     = icqsound

USETHREAD   = 1
MAKEDLL	    = 1

COMMON      = icqsound.c sndconfig.c scheme.c grpconfig.c

ifdef OS2

   EXETYPE     = VIO
   EXTRALIB    = mmpm2.lib \public\pwicq\os2\lib\icqkrnl.lib
   EXTRAINCL   = w:\data\ibmcpp\sources\icq2\include

   FLAGFILE    = w:\private\pwicq2\builds.txt
   EXEPATH     = w:\private\pwicq2\plugins
   RELEASEPATH = w:\public\pwicq\os2\bin\plugins

   RCFILE      = icqsound.rc
   SOURCES     = $(COMMON) os2play.c
   EXEC	       = go.cmd

else

   GNOME-AUDIO = 1

endif


ifdef GNOME-AUDIO

   CPARMS      = -DGNOMEAUDIO -DUSEICQLIB

   EXTRAINCL   = /usr/include/gnome-1.0 /usr/include/gtk-1.2 /usr/include/glib-1.2 /usr/lib/glib/include \
                 /usr/X11R6/include /home/perry/include /home/perry/include/pwicq  /usr/lib/gnome-libs/include \
                 ../../include

   EXTRALIB    = gnome gnomesupport esd audiofile m db1 glib

   EXEPATH     = /usr/share/pwicq/plugins
   SOURCES     = $(COMMON) pwicqlib.c lnxplay.c

endif

ifdef LINUX
   EXEC        = /usr/share/pwicq/pwicq --uin=6838492 --plugins=0 --module=$(PWD)/$(PROJECT).so --nodaemon --modules=icqgtk2,icqconf,icqusrdb
endif

ifdef PROJECT
   include pwMake.gnu
endif

