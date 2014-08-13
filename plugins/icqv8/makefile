
#---[ Dados do projeto ]---------------------------------------------------------------#

PROJECT   = icqv8
USETHREAD = 1
MAKEDLL   = 1

COMMON    = icqv8.c main.c modes.c tables.c tools.c channel1.c packet.c channel4.c extract.c channel2.c \
            snac01.c snac02.c snac03.c snac04.c snac09.c snac13.c userlist.c login.c snac15.c  snac0b.c \
            message.c advanced.c exec.c meta.c config.c tlv2711.c result.c sdialog.c rcvgreet.c passwd.c

ifdef OS2
   EXETYPE     = VIO
   CPARMS      = -DEXTENDED_LOG -DLOGUSER=27241234 -DLOGPACKETS
   EXTRALIB    = \public\pwicq\os2\lib\icqkrnl.lib \public\pwicq\os2\lib\icqnetw.lib
   DEFFILE     = icqv8.def

   EXTRAINCL   = w:\data\ibmcpp\sources\icq2\include
   FLAGFILE    = w:\private\pwicq2\builds.txt

   EXEPATH     = w:\private\pwicq2\plugins
   RELEASEPATH = w:\public\pwicq\os2\bin\plugins

   RCFILE      = icqv8.rc

   SOURCES     = $(COMMON)

   EXEC        = go.cmd

else

   CPARMS      = -DEXTENDED_LOG -DLOGUSER=27241234 -DLOGPACKETS -DUSEICQLIB
   EXTRAINCL   = ../../include
   EXEPATH     = /usr/share/pwicq/plugins
   EXTRALIB    = icqnetw
   EXTRAOBJ    = $(HOME)/public/pwicq/linux/lib
   SOURCES     = $(COMMON)
   EXEC        = /usr/share/pwicq/pwicq-loader --uin=6838492 --plugins=0 --module=$(PWD)/icqv8.so --nodaemon --module=/usr/share/pwicq/plugins/icqgtk.so --module=/usr/share/pwicq/plugins/icqusrdb.so --module=/usr/share/pwicq/plugins/icqconf.so
   MAKERPM     = icqv8.spec

endif

include pwMake.gnu

