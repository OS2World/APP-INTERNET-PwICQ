
#---[ Dados do projeto ]---------------------------------------------------------------#

PROJECT   = icqcmd

MAKEEXE   = 1

ifdef OS2

   EXETYPE     = VIO
   DEFFILE     = icqcmd.def
   EXTRAINCL   = w:\data\ibmcpp\sources\icq2\include
   EXEPATH     = w:\private\pwicq2
   SOURCES     = icqcmd.c
   EXTRALIB    = \public\pwicq\os2\lib\icqipc.lib
   RELEASEPATH = \public\pwicq\os2
else

   EXTRAINCL   = ../../include
   MAKERPM     = icqcmd.spec
   EXEPATH     = /usr/share/pwicq
   SOURCES     = queuecmd.c icqcmd.c lnxqueue.c
   EXEC        = ./icqcmd to 6838492 display http://www.terra.com.br

endif

#---[ Diretorios ]---------------------------------------------------------------------#

include pwMake.gnu




