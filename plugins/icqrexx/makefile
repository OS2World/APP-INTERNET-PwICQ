
#---[ Dados do projeto ]---------------------------------------------------------------#

PROJECT     = icqrexx
USETHREAD   = 1
MAKEDLL	    = 1

COMMON      = icqrexx.c evtconf.c system.c user.c

ifdef OS2
   EXETYPE     = VIO
   EXTRALIB    = \public\pwicq\os2\lib\icqkrnl.lib
   EXTRAINCL   = w:\data\ibmcpp\sources\icq2\include

   FLAGFILE    = w:\private\pwicq2\builds.txt

   EXEPATH     = w:\private\pwicq2\plugins
   RELEASEPATH = w:\public\pwicq\os2\bin\plugins

   RCFILE      = icqrexx.rc
   SOURCES     = $(COMMON) os2start.c 

   EXEC        = go.cmd
   
else   
   CPARMS      = -DEXTENDED_LOG
   EXTRAINCL   = ../../include
#   EXTRALIB    = icqnetw
   MAKERPM     = icqrexx.spec
   CPARMS      = -DUSEICQLIB
   EXEPATH     = /usr/share/pwicq/plugins
   SOURCES     = $(COMMON) pwicqlib.c lnxstart.c
#   EXEC        = /usr/share/pwicq/pwicq --uin=6838492 --inifile=$(HOME)/.pwicq/pwicq.conf --plugins=0 --text --module=icqrexx.so
   EXEC        = /usr/share/pwicq/pwicq --uin=6838492 --inifile=$(HOME)/.pwicq/pwicq.conf --plugins=0 --module=icqrexx.so --module=/usr/share/pwicq/plugins/icqgtk.so --module=/usr/share/pwicq/plugins/icqconf.so --nodaemon
endif

include pwMake.gnu

