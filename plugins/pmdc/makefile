
#---[ Dados do projeto ]---------------------------------------------------------------#

PROJECT     = icqpmdc
USETHREAD   = 1
MAKEDLL     = 1
CPARMS      = -DLOG_ALL

ifdef OS2

   EXETYPE     = VIO
   EXTRALIB    = \public\pwicq\os2\lib\pwicq2.lib
   EXTRAINCL   = w:\data\ibmcpp\sources\icq2\include

   EXEPATH     = w:\private\pwicq2\plugins
   RELEASEPATH = w:\public\pwicq\os2
   FLAGFILE    = w:\private\pwicq2\builds.txt
   RCFILE      = pmdc.rc

   SOURCES     = icqpmdc.c config.c os2pm.c

   EXEC        = go.cmd

   include pwMake.gnu

endif






