
#---[ Dados do projeto ]---------------------------------------------------------------#

PROJECT   = icqipc

BASE      = queuecmd.c rxapi.c rxbase.c

ifdef DEBUG
   MAKEEXE   = 1
endif


ifdef MAKEEXE
   DEFFILE   = ipctest.def
else
   MAKEDLL   = 1
   USETHREAD = 1
   DEFFILE   = icqipc.def
endif

ifdef OS2

   EXETYPE     = VIO

   SAVELIB     = w:\public\pwicq\os2\lib
   EXTRAINCL   = w:\data\ibmcpp\sources\icq2\include
   EXTRALIB    = rexx.lib tcpip32.lib

   EXEPATH     = w:\private\pwicq2
   RELEASEPATH = s:\bin\dll
   FLAGFILE    = w:\private\pwicq2\builds.txt

   SRC         = os2queue.c

else
   MAKERPM     = ../../pwicq2.spec

   EXTRAINCL   = ../../include /usr/local/orexx

   EXEPATH     = /usr/share/pwicq
   SAVELIB     = /mnt/os2data/linux/lib
   EXTRALIB    = rexx
   MAKERPM     = ipcicq.spec

   SRC         = lnxqueue.c
#   EXEC        = go

endif

ifdef MAKEEXE
   SOURCES = $(SRC) $(BASE) load.c
else
   SOURCES = $(SRC) $(BASE)
endif


#---[ Diretorios ]---------------------------------------------------------------------#

include pwMake.gnu




