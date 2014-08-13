
#---[ Dados do projeto ]---------------------------------------------------------------#

ifdef LINUX

   PROJECT    = icqosd

   USETHREAD  = 1
   MAKEDLL    = 1
   MAKEBACKUP = 1
   EXTRAINCL  = ../../include

   SOURCES    = icqosd.c
   CPARMS     = -DUSEICQLIB 
   EXEPATH    = /usr/share/pwicq/plugins
   
   EXTRALIB   = xosd
   EXTRAOBJ   = $(HOME)/lib/libicqplugin.a
   
   MAKERPM     = icqosd.spec
   EXEC        = /usr/bin/pwicq --uin=6838492 --plugins=0 --offline \
                 --nodaemon \
                 --modules=icqusrdb,icqmsg,icqv8,icqconf,icqgtk2 \
                 --module=$(PWD)/$(PROJECT).so


   include pwmake.gnu
   
endif
