
#---[ Dados do projeto ]---------------------------------------------------------------#

ifdef LINUX

   PROJECT    = icqgtk

   USETHREAD  = 1
   MAKEEXE    = 1
   MAKEBACKUP = 1
   MAKERPM    = $(HOME)/icq2/pwicq2.spec

   BASE       = mainwin.c main.c user.c system.c actions.c timer.c msgdlg.c misc.c \
       	        uknformat.c event.c login.c cfgwin.c table.c lnxconfig.c

   EXTRAINCL =  /usr/include/gnome-1.0 /usr/lib/gnome-libs/include /usr/include/gtk-1.2 /usr/include/glib-1.2 \
                /usr/lib/glib/include /usr/X11R6/include /usr/include/gnome-1.0 /usr/lib/gnome-libs/include \
                /usr/include/gtk-1.2 /usr/include/gnome-1.0 \
                /usr/include/gdk-pixbuf-1.0 \
                /home/perry/include ../../include


   ifdef MAKEEXE
      EXTRALIB  = gthread pthread gdk_pixbuf gtk gdk gmodule glib dl Xi Xext X11 m icqkrnl 
###                  gnome gnomesupport esd audiofile db1
      SOURCES   = lnxloader.c $(BASE)
      EXEPATH   = $(HOME)/public/pwicq2/linux/bin
   else
      EXTRALIB  = gthread pthread gdk_pixbuf gtk gdk gmodule glib dl Xi Xext X11 m
      SOURCES   = pwicqlib.c $(BASE)
      EXEPATH   = $(HOME)/public/pwicq2/linux/bin/plugins
   endif   

   LIBPATH   = /usr/X11R6/lib

   #CPARMS    = -DEXTENDED_LOG -DDEBUG_LOG

   ifdef MAKEEXE
   #   EXEC   = $(PROJECT) --uin=6838492 --inifile=/home/perry/icq2/config/pwicq.conf --config-files=/home/perry/icq2/config/ --plugins=1
      EXEC   = $(PROJECT) --uin=6838492 --nodaemon --plugins=1
   endif     

   MAKERPM = ../../pwicq2.spec

   include pwmake.gnu
   
endif
