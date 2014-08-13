
#---[ Dados do projeto ]---------------------------------------------------------------#

PROJECT     = icqmsg
USETHREAD   = 1
MAKEDLL     = 1
CPARMS      = -DEXTENDED_LOG

SOURCES     = icqmsg.c msg.c url.c authorize.c request.c refuse.c added.c file.c

ifdef OS2
   EXETYPE     = VIO
   EXTRALIB    = \public\pwicq\os2\lib\icqkrnl.lib
   EXTRAINCL   = w:\data\ibmcpp\sources\icq2\include
   FLAGFILE    = w:\private\pwicq2\builds.txt

   EXEPATH     = w:\private\pwicq2\plugins
   RELEASEPATH = w:\public\pwicq\os2\bin\plugins

   RCFILE      = icqmsg.rc

   EXEC        = go.cmd

else
   EXTRAINCL   = ../../include
   CPARMS      = -DUSEICQLIB
   EXEPATH     = /usr/share/pwicq/plugins
   EXTRAOBJ    = /mnt/os2data/linux/lib/libicqplugin.a
   MAKERPM     = icqmsg.spec
   EXEC        = /usr/share/pwicq/pwicq.sh --uin=6838492 --inifile=$(HOME)/.pwicq/pwicq.conf --plugins=0 --modules=icqmsg,icqusrdb,icqgtk2 --module=$(PWD)/icqmsg.so --nodaemon

endif

include pwMake.gnu

