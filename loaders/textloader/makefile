
#---[ Dados do projeto ]---------------------------------------------------------------#

PROJECT     = icqtext
MAKEEXE     = 1
USETHREAD   = 1
SOURCES     = icqtext.c

ifdef OS2
   EXETYPE     = VIO
   EXTRALIB    = \public\pwicq\os2\lib\icqkrnl.lib

   EXTRAINCL   = \data\ibmcpp\sources\icq2\include
   EXEPATH     = w:\private\pwicq2
   RELEASEPATH = w:\public\pwicq\os2\bin
   EXEC        = go.cmd

   include pwMake.gnu

else

   #---[ ICQTEXT is not beeing used under linux ]----------------------------------------#

   EXTRAINCL   = $(HOME)/include/pwicq
   EXEPATH     = $(HOME)/public/pwicq2/linux/bin
   EXTRALIB    = icqkrnl
   EXEC        = go

endif


# gcc -o $(EXEPATH)/pwicqt $(LOADER:.c=.o) -L$(LIBPATH) -lpwicq -ldl -lpthread



