
#---[ Dados do projeto ]---------------------------------------------------------------#

PROJECT   = icqconf
USETHREAD = 1
MAKEDLL   = 1
CPARMS    = -DEXTENDED_LOG
COMMON    = icqconf.c spmcfg.c groups.c away.c reqcfg.c misc.c basic.c
RCFILE    = icqconf.rc

ifdef OS2
   EXETYPE     = VIO
   DEFFILE     = icqconf.def

   EXTRAINCL   = w:\data\ibmcpp\sources\icq2\include
   EXTRALIB    = \public\pwicq\os2\lib\icqkrnl.lib


   FLAGFILE    = w:\private\pwicq2\builds.txt

   EXEPATH     = w:\private\pwicq2\plugins
   RELEASEPATH = w:\public\pwicq\os2\bin\plugins

   SOURCES     = $(COMMON) cfgcpg.c

   EXEC        = go.cmd

else
   EXTRAINCL   = ../../include
   EXEPATH     = /usr/share/pwicq/plugins
   EXTRAOBJ    = $(HOME)/lib/libicqplugin.a
   CPARMS      = -DUSEICQLIB

   SOURCES     = $(COMMON)
   EXEC        = /usr/share/pwicq/pwicq --uin=6838492 --inifile=/home/perry/.pwicq/pwicq.conf --plugins=0 --module=$(PWD)/icqconf.so --nodaemon --modules=icqgtk2,icqusrdb,icqv8

endif


include pwMake.gnu

