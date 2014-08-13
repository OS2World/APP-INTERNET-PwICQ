
#---[ Flags de montagem ]--------------------------------------------------------------#

PWICQ     = 1
CLIENT    = 1

ifndef PWICQ
   STDALONE  = 1
endif

#---[ Fontes (separados por grupo ]----------------------------------------------------#

COMMON    = main.c send.c search.c timer.c crc16.c list.c init.c users.c md5.c
CLIENT    = ftrans.c cfile.c tcp.c tcptools.c sndfile.c rcvfile.c
ADMIN     = adm.c

#---[ Flags de montagem ]--------------------------------------------------------------#

NETFLG = -DUSEICQNETW

ifdef PWICQ

   PROJECT   = isicq
   MAKEDLL   = 1
   USETHREAD = 1
   DEFFILE   = isicq.def

   LOADER    = pwicqplg.c exec.c pwicqmsg.c
   LDRFLG    = -DPWICQ

endif

ifdef STDALONE
   PROJECT   = ishare
   MAKEEXE   = 1
   USETHREAD = 1
   DEFFILE   = ishare.def
   LOADER    = ishare.c config.c
endif

ifdef CLIENT
   FLGSRV    = -DCLIENT
endif

BASEPARMS = -DEXTENDED_LOG $(FLGSRV) -DADMIN -DLOGSEARCHS $(LDRFLG) $(NETFLG)

ifdef OS2
   EXETYPE     = VIO
ifdef PWICQ
   EXTRALIB    = \public\pwicq\os2\lib\icqkrnl.lib \public\pwicq\os2\lib\icqnetw.lib
endif
ifdef STDALONE
   EXTRALIB    = \public\pwicq\os2\lib\icqnetw.lib
endif

   EXTRAINCL   = w:\data\ibmcpp\sources\icq2\include
   FLAGFILE    = w:\private\pwicq2\builds.txt
   CPARMS      = $(BASEPARMS)

   EXEPATH     = w:\private\pwicq2\plugins
   RELEASEPATH = w:\public\pwicq\os2
   ZIPSOURCE   = w:\private\i-share
#   RCFILE      = isicq.rc

   SOURCES     = $(COMMON) $(ADMIN) $(CLIENT) $(LOADER) os2fldr.c tools.c

   EXEC        = go.cmd

else

   EXTRAINCL   = ../../include
   EXTRALIB    = icqnetw
   EXEPATH     = /usr/share/pwicq/plugins
ifdef PWICQ
   SOURCES     = $(COMMON) $(ADMIN) $(CLIENT) $(LOADER) pwicqlib.c lnxfldr.c tools.c
#   MAKERPM     = isicq.spec
else
   SOURCES     = $(COMMON) $(ADMIN) $(CLIENT) $(LOADER) lnxfldr.c tools.c
   MAKERPM     = ishared.spec
endif
   CPARMS      = $(BASEPARMS) -DUSEICQLIB
   EXEC        = /usr/share/pwicq/pwicq --uin=6838492 --text --plugins=0 --module=$(PWD)/isicq.so --nodaemon --modules=icqv8p,icqusrdb
   # --modules=icqgtk2,icqconf,icqv8


endif

include pwMake.gnu

