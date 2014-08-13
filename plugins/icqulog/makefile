
#---[ Dados do projeto ]---------------------------------------------------------------#

PROJECT     = icqulog
USETHREAD   = 1
MAKEDLL     = 1

COMMON      = icqulog.c

ifdef OS2
   EXETYPE     = VIO
   EXTRALIB    = \public\pwicq\os2\lib\icqkrnl.lib
   EXTRAINCL   = w:\data\ibmcpp\sources\icq2\include
   FLAGFILE    = w:\private\pwicq2\builds.txt

   EXEPATH     = w:\private\pwicq2\plugins
   RELEASEPATH = w:\public\pwicq\os2\bin\plugins

#   RCFILE      = icqulog.rc
   SOURCES     = $(COMMON)
   FLAGFILE    = w:\private\pwicq2\builds.txt

   EXEC        = go.cmd

else
   EXTRAINCL   = ../../include
   EXEPATH     = /usr/share/pwicq/plugins
   SOURCES     = $(COMMON)
   EXTRAOBJ    = /mnt/os2data/linux/lib/libicqplugin.a
   MAKERPM     = ../../core/kernel/icqcore.spec
   CPARMS      = -DUSEICQLIB
   EXEC        = /usr/share/pwicq/pwicq-loader --uin=6838492 --inifile=$(HOME)/.pwicq/pwicq.conf --plugins=0 --modules=icqgtk2,icqusrdb --module=$(PWD)/icqulog.so --nodaemon

endif

include pwMake.gnu
