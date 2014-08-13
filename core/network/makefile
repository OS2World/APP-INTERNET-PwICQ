
#---[ Dados do projeto ]---------------------------------------------------------------#


PROJECT   = icqnetw
USETHREAD = 1
MAKEDLL   = 1
#TCP16     = 1

ifdef TCP16
   TCPPARMS = -DEXTENDED_LOG -DTCPV40HDRS
endif

COMMON    = common.c

ifdef OS2
   EXETYPE      = VIO
   TARGETS      = SCK.MSG
#   CPARMS       = -DOS2SOCKS=1 $(TCPPARMS)
   CPARMS       = $(TCPPARMS)

   ifdef TCP16
      EXTRALIB    = tcp32dll.lib so32dll.lib
      EXEPATH     = w:\public\pwicq2\os2
   else
      EXTRALIB    = tcpip32.lib
      EXEPATH     = w:\private\pwicq2
      RELEASEPATH = w:\public\pwicq\os2\bin
   endif

   DEFFILE      = icqnet.def

   EXTRAINCL    = w:\data\ibmcpp\sources\icq2\include
   SAVELIB      = w:\public\pwicq\os2\lib
   FLAGFILE     = w:\private\pwicq2\builds.txt
   EXEC		= go.cmd

   SOURCES      = $(COMMON) os2net.c
endif

ifdef WIN32
   SOURCES      = $(COMMON) winnet.c
   SAVELIB      = $(PROJECT).lib
   ZIPFILE	= w:\Private\I-Share\icqnetw_win.zip
endif

ifdef LINUX
   EXTRAINCL    = ../../include
   SAVELIB      = /mnt/os2data/linux/lib
   EXEPATH      = /usr/share/pwicq
   MAKERPM      = ../../pwicq2.spec
   SOURCES      = $(COMMON) lnxnet.c
endif

include pwMake.gnu
