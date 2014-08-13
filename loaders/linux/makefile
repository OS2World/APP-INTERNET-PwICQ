
#---[ Dados do projeto ]---------------------------------------------------------------#

PROJECT     = pwicq
MAKEEXE     = 1
USETHREAD   = 1

SOURCES     = lnxloader.c pwicqlib.c

CPARMS      = -DPWICQ_DINAMIC -DUSEICQLIB

ifdef LINUX
   EXTRAINCL   = ../../include
   EXEPATH     = /usr/share/pwicq
   EXTRALIB    = dl
   EXEC	       = ./pwicq --uin=6838492 --plugins=0 --text
   MAKERPM     = ../../core/kernel/icqcore.spec
   
   include pwMake.gnu
endif




