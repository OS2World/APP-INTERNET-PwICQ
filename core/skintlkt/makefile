
#---[ Dados do projeto ]---------------------------------------------------------------#

USETHREAD = 1

COMMON    = menu.c listbox.c defwinp.c slider.c skmain.c bitmap.c btnclass.c load.c \
            register.c create.c sizemain.c control.c images.c

#CPARMS   = -DDEBUG_LOG
#CPARMS    = -DEXTENDED_LOG

ifndef DEBUG

   #---[ Sem DEBUG - Monta DLL ]-----#
   PROJECT     = icqskin
   MAKEDLL     = 1
   EXETYPE     = VIO
   LOADER      = dllmain.c
   EXEPATH     = w:\private\pwicq2
   SAVELIB     = w:\public\pwicq\os2\lib
   EXEPATH     = w:\private\pwicq2
   RELEASEPATH = w:\public\pwicq\os2\bin
   FLAGFILE    = w:\private\pwicq2\builds.txt

else

   #----[ DEBUG - Monta EXE ]-------#
   PROJECT     = skintest
   EXETYPE     = PM
   MAKEEXE     = 1
   LOADER      = loader.c
   RCFILE      = skintest.rc
   EXEPATH     = w:\temp
   EXEC        = go.cmd

endif

EXTRAINCL = w:\data\ibmcpp\sources\icq2\include
EXTRALIB  = mmpm2.lib
SOURCES   = $(LOADER) $(COMMON)

#---[ Diretorios ]---------------------------------------------------------------------#

include pwMake.gnu




