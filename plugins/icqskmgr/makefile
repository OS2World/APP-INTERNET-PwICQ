
#---[ Dados do projeto ]---------------------------------------------------------------#

PROJECT     = icqskmgr
USETHREAD   = 1
MAKEDLL     = 1
CPARMS      = -DEXTENDED_LOG

ifdef OS2
   EXETYPE     = VIO
   EXTRALIB    = \public\pwicq\os2\lib\icqkrnl.lib \public\pwicq\os2\lib\icqskin.lib \public\pwicq\os2\lib\icqwin.lib
   EXTRAINCL   = w:\data\ibmcpp\sources\icq2\include

   EXEPATH     = w:\private\pwicq2\plugins
   RELEASEPATH = w:\public\pwicq\os2\bin\plugins

   FLAGFILE    = w:\private\pwicq2\builds.txt

   RCFILE      = icqskmgr.rc
   SOURCES     =  icqskmgr.c init.c mainwin.c sysevent.c sysmenu.c timer.c ulist.c \
                  userevent.c usrmenu.c away.c config.c drag.c

   EXEC        = go.cmd

   include pwMake.gnu

endif






