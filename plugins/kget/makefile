
#---[ Dados do projeto ]---------------------------------------------------------------#

ifdef LINUX

   PROJECT    = icqkget

   USETHREAD  = 1
   MAKEDLL    = 1
   MAKEBACKUP = 1
   EXTRAINCL  = ../../include

   SOURCES    = icqkget.c pwicqlib.c
   CPARMS     = -DUSEICQLIB 
   EXEPATH    = /usr/share/pwicq/plugins

#   MAKERPM     = icqkget.spec
   EXEC        = /usr/share/pwicq/pwicq-loader --uin=6838492 --offline --inifile=$(HOME)/.pwicq/pwicq.conf --plugins=0 --nodaemon --modules=icqusrdb,icqgtk2,icqmsg --module=$(PWD)/$(PROJECT).so 

   include pwmake.gnu
   
endif
