
#---[ Dados do projeto ]---------------------------------------------------------------#

PROJECT     = pwICQ2
MAKEEXE     = 1
USETHREAD   = 1

ifdef OS2
   EXETYPE     = PM
   EXTRALIB    = \public\pwicq\os2\lib\icqkrnl.lib

   EXTRAINCL   = \data\ibmcpp\sources\icq2\include
   EXEPATH     = w:\private\pwicq2
   RELEASEPATH = w:\public\pwicq\os2\bin

   RCFILE      = pwicq.rc
   SOURCES     = pwicq.c
   FLAGFILE    = w:\private\pwicq2\builds.txt
   EXEC        = go.cmd

   include pwMake.gnu

endif





