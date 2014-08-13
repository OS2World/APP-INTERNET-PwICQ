
#---[ Dados do projeto ]---------------------------------------------------------------#

USETHREAD    = 1
MAKEDLL      = 1

#GNOME_APPLET = 1
PIPENAME     = "icq.pipe"

COMMON       = icqpipe.c config.c

ifdef OS2

   PROJECT     = icqpipe

   EXETYPE     = VIO
   EXTRALIB    = \public\pwicq\os2\lib\pwicq2.lib
   EXTRAINCL   = w:\data\ibmcpp\sources\icq2\include

   EXEPATH     = w:\private\pwicq2\plugins
   RELEASEPATH = w:\public\pwicq\os2\bin\plugins

   RCFILE      = icqpipe.rc
   SOURCES     = $(COMMON) os2pipe.c
   FLAGFILE    = w:\private\pwicq2\builds.txt

else
   EXTRAINCL   = ../../include
   EXEPATH     = /mnt/os2data/public/pwicq/linux/experimental

   ifdef GNOME_APPLET
      #---[ If building a Gnome Applet ]----------------------------------------------#
      PROJECT     = icqgpipe
      CPARMS      = -DGNOME_APPLET -DUSEICQLIB
      SOURCES     = $(COMMON) gnomeapplet.c pwicqlib.c
#      MAKERPM     = $(HOME)/icq2/plugins/icqpipe/icqgnome.spec

   endif

   ifdef PIPENAME
      #---[ If building a pipewriter plugin ]-----------------------------------------#
      PROJECT     = icqpipe
      CPARMS	  = -DPIPENAME=$(PIPENAME) -DUSEICQLIB
      SOURCES     = $(COMMON) lnxpipe.c pwicqlib.c
#      MAKERPM     = $(HOME)/icq2/plugins/icqpipe/icqpipe.spec
   endif


endif

ifdef SOURCES
   include pwMake.gnu
endif




