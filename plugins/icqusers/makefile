
#---[ Dados do projeto ]---------------------------------------------------------------#

PROJECT   = icqusrdb
USETHREAD = 1
MAKEDLL   = 1
COMMON    = userdb.c main.c userlist.c

ifdef OS2
   EXETYPE     = VIO

   EXTRAINCL   = w:\data\ibmcpp\sources\icq2\include
   EXTRALIB    = \public\pwicq\os2\lib\icqkrnl.lib

   FLAGFILE    = w:\private\pwicq2\builds.txt

   EXEPATH     = w:\private\pwicq2\plugins
   RELEASEPATH = w:\public\pwicq\os2\bin\plugins
   SOURCES     = $(COMMON)

   EXEC        = go.cmd


else
   EXTRAINCL   = ../../include
   EXEPATH     = /usr/share/pwicq/plugins
   EXTRAOBJ    = /mnt/os2data/linux/lib/libicqplugin.a
   MAKERPM     = icqcore.spec
   CPARMS      = -DUSEICQLIB
   SOURCES     = $(COMMON) pwicqlib.c
   EXEC        = /usr/share/pwicq/pwicq --text --uin=6838492 --nodaemon --plugins=0 --module=/mnt/os2data/data/ibmcpp/sources/icq2/plugins/icqusers/icqusrdb.so

endif


include pwMake.gnu

