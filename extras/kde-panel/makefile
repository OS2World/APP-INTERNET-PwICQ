
#---[ Dados do projeto ]---------------------------------------------------------------#

PROJECT    = pwicqkde_applet

ifdef LINUX

   USETHREAD  = 1
   MAKEDSO    = 1
   MAKEBACKUP = 1
   EXEPATH    = /usr/share/pwicq
   LIBTOOL    = 1

   LIBPATH     = /usr/lib/qt3/lib \
                 /usr/X11R6/lib

   EXTRAINCL   = /usr/lib/qt3/include \
                 /home/perry/include ../../include \
	             /usr/X11R6/include


   EXTRALIB    = kdeui kdecore kio qt-mt  png z m Xext X11 resolv SM ICE icqipc

   MAKERPM     = kde-panel.spec

   MOCFILES   = kdepanel.h
   CPPSOURCES = kdepanel.cc 
#   EXEC       = go

   include pwMake.gnu

endif
