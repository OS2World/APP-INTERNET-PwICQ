
#---[ Dados do projeto ]---------------------------------------------------------------#

   PROJECT     = gtktest
   USETHREAD   = 1
   MAKEEXE     = 1
   MAKEBACKUP  = 1

   EXTRAINCL   = ../../include
   SOURCES     = gtkloader.c gtkframe.c gtklabel.c gtkbutton.c
   
   # pkg-config --cflags gtk+-2.0
   CPARMS      = -DEXTENDED_LOG -DGTK2 -DSKINNED_GUI                 \
                 -I/usr/include/gtk-2.0  -I/usr/lib/gtk-2.0/include  \
				 -I/usr/include/atk-1.0  -I/usr/include/pango-1.0    \
				 -I/usr/X11R6/include    -I/usr/include/freetype2    \
				 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include


   # pkg-config --libs gtk+-2.0
   LDFLAGS     = -lgtk-x11-2.0    -lgdk-x11-2.0    -latk-1.0      \
                 -lgdk_pixbuf-2.0 -lm              -lpangoxft-1.0 \
				 -lpangox-1.0     -lpango-1.0      -lgobject-2.0  \
				 -lgmodule-2.0    -ldl             -lglib-2.0     \
				 -lgthread-2.0


  include pwMake.gnu

