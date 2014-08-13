
#---[ Dados do projeto ]---------------------------------------------------------------#

USETHREAD = 1

COMMON    = timer.c log.c init.c user.c list.c register.c config.c \
            modes.c event.c plugin.c msg.c uquery.c tfile.c \
            query.c blacklist.c setvals.c tools.c svlist.c away.c \
            ftable.c qproc.c rtf.c badword.c exec.c search.c \
            listener.c srvcalls.c peer.c request.c menu.c

CPARMS    = -DEXTENDED_LOG

ifdef OS2

   PROJECT     = icqkrnl
   MAKEDLL     = 1

   EXETYPE     = VIO
   DEFFILE     = os2core.def

   PWLIB       = w:\public\pwicq\os2\lib
   EXTRAINCL   = w:\data\ibmcpp\sources\icq2\include
   EXTRALIB    = \public\pwicq\os2\lib\icqnetw.lib

   EXEPATH     = w:\private\pwicq2
   RELEASEPATH = w:\public\pwicq\os2\bin
   FLAGFILE    = w:\private\pwicq2\builds.txt

   OS          = os2plug.c os2thread.c os2config.c os2start.c os2query.c os2url.c os2tools.c
   EXEC        = go.cmd

else

   ifdef DEBUG
      MAKEEXE     = 1
      EXEPATH     = $(HOME)/tmp
      EXEC        = ./pwicq --uin=6838492 --plugins=0 --text --nodaemon 
   else
      MAKEDLL     = 1
      EXEPATH     = /usr/share/pwicq
      EXEC        = /usr/share/pwicq/pwicq --uin=6838492 --plugins=1 --nodaemon --module=/mnt/os2data/public/pwicq/linux/experimental/icqgtk2.so
   endif

   MAKERPM     = icqcore.spec

   EXTRAINCL   = ../../include

   EXTRALIB    = dl icqnetw
   PWLIB       = /mnt/os2data/linux/lib

   LOADER      = lnxloader.c
   OS          = lnxconf.c  lnxplug.c  lnxstart.c  lnxthread.c lnxurl.c cmdline.c lnxquery.c \
                 lnxresource.c

   MAKELIB     =  $(OS) $(COMMON)

endif

ifdef MAKEEXE
   PROJECT = pwicq
   SOURCES = $(LOADER) $(OS) $(COMMON)
endif

ifdef MAKEDLL
   PROJECT = icqkrnl
   SAVELIB = $(PWLIB)
   SOURCES = $(OS) $(COMMON)
endif

#---[ Diretorios ]---------------------------------------------------------------------#

include pwMake.gnu




