
#---[ Dados do projeto ]---------------------------------------------------------------#

USETHREAD   = 1
MAKEDLL     = 1

#ifdef OS2
#   MAKELIB=1
#   CLASSIC = 1
#endif

COMMON      = tables.c actions.c load.c event.c tools.c ulevent.c \
              bitmap.c msg.c msgevent.c edit.c buttons.c menu.c   \
              config.c cfgevent.c


ifdef OS2

   ifdef MAKELIB
      PROJECT     = icqwin
      CPARMS      = -DEXTENDED_LOG -DSTANDARD_GUI -DEXTENDED_LOG -DMAKELIB=1 -DUSE_MMPM
      EXTRALIB    = \public\pwicq\os2\lib\icqkrnl.lib mmpm2.lib
      RCFILE      = newskin.rc
   else
      ifdef CLASSIC
         PROJECT  = classic
         CPARMS   = -DSTANDARD_GUI -DICONTABLE16 -DGUINAME=\"Classic\"
         EXTRALIB = \public\pwicq\os2\lib\icqkrnl.lib
         RCFILE   = classic.rc
   else
         PROJECT = icqgui
         CPARMS  = -DEXTENDED_LOG -DSKINNED_GUI -DUSE_MMPM
         EXTRALIB = \public\pwicq\os2\lib\icqkrnl.lib mmpm2.lib
         RCFILE      = newskin.rc
   endif
   endif

   EXETYPE     = VIO

   EXTRAINCL   = w:\data\ibmcpp\sources\icq2\include

   EXEPATH     = w:\temp

   FLAGFILE    = w:\private\pwicq2\builds.txt

   ifdef MAKELIB
      EXEC        = golib.cmd
      BASE        = libmain.c
      EXEPATH     = w:\private\pwicq2
      RELEASEPATH = w:\public\pwicq\os2\bin
      SAVELIB     = w:\public\pwicq\os2\lib
   else
      EXEC        = go.cmd $(PROJECT)
      BASE        = main.c os2win.c
      RELEASEPATH = w:\Public\pwicq\os2\experimental
   endif

   OSDEPENDENT = os2tools.c os2create.c os2frame.c os2bitmap.c os2button.c os2bg.c \
                 os2ulist.c os2menu.c os2scbar.c os2static.c os2chatwin.c os2pagerwin.c \
                 os2config.c os2login.c os2dialog.c os2dde.c os2upopup.c


endif

ifdef LINUX

#   GTK1        = 1
   ifndef GTK1
      GTK2 = 1
   endif

   BASE        = main.c
   EXEPATH     = /usr/share/pwicq/plugins
   RCFILE      = newskin.rc

   EXEC        = /usr/bin/pwicq --uin=6838492 --plugins=0 --offline \
                 --nodaemon \
				 --modules=icqusrdb,icqmsg,icqconf \
				 --module=$(PWD)/$(PROJECT).so
				
   EXTRAINCL   = ../../include
endif

ifdef GTK2

   PROJECT     = icqgtk2
   EXEPATH     = /usr/share/pwicq/plugins
   MAKERPM     = gtk2.spec

   CPARMS      = -DEXTENDED_LOG -DGTK2 -DSTANDARD_GUI -DUSEICQLIB    \
   				 `pkg-config --cflags gtk+-2.0`			
				
   LDFLAGS     = `pkg-config --libs gtk+-2.0 gthread-2.0`

   OSDEPENDENT = pwicqlib.c icqgtk.c gtknoskin.c gtktools.c \
                 gtkbutton.c gtkulist.c gtkmenu.c gtkchatwin.c gtkframe.c \
				 gtkpagerwin.c gtkconfig.c gtkdialog.c gtklabel.c \
				 gtkmsgdialog.c gtkupopup.c gtkbitmap.c

endif

ifdef GTK1

   PROJECT     = icqgtk1
   EXEPATH     = /mnt/os2data/public/pwicq/linux/experimental
   MAKERPM     = gtk2.spec

   CPARMS      = -DEXTENDED_LOG -DGTK1 -DSTANDARD_GUI -DUSEICQLIB \
				 -I/usr/include/gdk-pixbuf-1.0                    \
                 $(LIBFLAGS) `gtk-config --cflags`

   LDFLAGS     =  `gtk-config --libs gtk gthread` -lgdk_pixbuf

   OSDEPENDENT = pwicqlib.c icqgtk.c gtknoskin.c gtktools.c \
                 gtkbutton.c gtkulist.c gtkmenu.c gtkchatwin.c gtkframe.c \
				 gtkpagerwin.c gtkconfig.c gtkdialog.c gtklabel.c \
				 gtkmsgdialog.c gtkupopup.c
				

endif

SOURCES = $(OSDEPENDENT) $(COMMON) $(BASE)

include pwMake.gnu
