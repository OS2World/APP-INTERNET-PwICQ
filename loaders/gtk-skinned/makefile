
#---[ Dados do projeto ]---------------------------------------------------------------#

ifdef LINUX

   PROJECT    = icqgtk2

   USETHREAD  = 1
   MAKEEXE    = 1
   MAKEBACKUP = 1

   SOURCES    = main.c init.c loadpixbuf.c

   EXTRAINCL =  /usr/include/gnome-1.0 /usr/lib/gnome-libs/include /usr/include/gtk-1.2 /usr/include/glib-1.2 \
                /usr/lib/glib/include /usr/X11R6/include /usr/include/gnome-1.0 /usr/lib/gnome-libs/include \
                /usr/include/gtk-1.2 /usr/include/gnome-1.0 \
                /usr/include/gdk-pixbuf-1.0 \
                /home/perry/include ../../include


   LIBPATH   = /usr/X11R6/lib
   EXTRALIB  = gthread pthread gdk_pixbuf gtk gdk gmodule glib dl Xi Xext X11 m
   EXEPATH   = $(HOME)/tmp

   include pwmake.gnu
   
endif
