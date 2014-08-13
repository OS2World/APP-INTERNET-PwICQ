

#---[ Dados do projeto ]---------------------------------------------------------------#

PROJECT   = icqv8p
USETHREAD = 1
MAKEDLL   = 1

COMMON    = main.c getset.c listen.c tcptools.c session.c exec.c connect.c sendmsg.c msg.c \
            encript.c rcvgreet.c sndgreet.c file.c request.c sendfile.c rcvfile.c

ifdef OS2
   EXETYPE     = VIO
   CPARMS      = -DEXTENDED_LOG
   EXTRALIB    = \public\pwicq\os2\lib\icqkrnl.lib \public\pwicq\os2\lib\icqnetw.lib
   DEFFILE     = icqv8p.def

   EXTRAINCL   = w:\data\ibmcpp\sources\icq2\include
   FLAGFILE    = w:\private\pwicq2\builds.txt

   EXEPATH     = w:\private\pwicq2\plugins
   RELEASEPATH = w:\public\pwicq\os2\bin\plugins

#  RCFILE      = icqv8.rc

   SOURCES     = $(COMMON) os2file.c

   EXEC        = go.cmd

else

   CPARMS      = -DEXTENDED_LOG -DUSEICQLIB
   EXTRAINCL   = ../../include
   EXEPATH     = /usr/share/pwicq/plugins
   EXTRALIB    = icqnetw
   EXTRAOBJ    = /mnt/os2data/linux/lib/libicqplugin.a
   SOURCES     = $(COMMON) lnxfile.c
   EXEC        = /usr/share/pwicq/pwicq --uin=6838492 --inifile=$(HOME)/.pwicq/pwicq.conf --plugins=0 --offline --text --module=$(PWD)/icqv8p.so --nodaemon 
   # --modules=icqusrdb,icqv8
   MAKERPM     = ../icqv8/icqv8.spec

endif

include pwMake.gnu

